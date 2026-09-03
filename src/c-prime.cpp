
#include <exception>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "lexer/lexer.cpp"
#include "parser/preparser.cpp"
#include "parser/parser.cpp"
#include "semantic/semantic.cpp"
#include "transpiler/cpp_emitter.cpp"
#include "cli/arguments.cpp"
#include "debug/print.cpp"
#include "io/file.cpp"

static void clearPhraseTrivia(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase) {
        return;
    }
    phrase->leading_trivia.clear();
    phrase->trailing_trivia.clear();
    phrase->trailing_comment.clear();
    for (const auto& nested : phrase->nested_phrases) {
        clearPhraseTrivia(nested);
    }
}

// How imported .cprime/.hprime files reach the output:
//   * `imports_dir` empty  -> their phrases are spliced (inlined) into the
//     importing file, and the whole program is emitted as one .cpp.
//   * `imports_dir` set     -> each imported file is transpiled into its own
//     .cpp under that directory and the importer `#include`s it; the shared
//     prelude is emitted once into `cprime-prelude.hpp` beside each file.
// Unless `import_all` is set, only the top-level declarations an importer
// actually uses (transitively) are carried over from an imported file.
struct TranspileState {
    bool debug = false;
    std::string imports_dir;
    bool import_all = false;
    ParsedPhrases prelude_phrases;
    std::string prelude_header_code;
    std::unordered_set<std::string> in_progress;
    std::unordered_map<std::string, std::string> transpiled;  // source path -> generated .cpp
    std::vector<std::string> generated_paths;                 // every generated dep .cpp
    // Phrases of every separately-transpiled unit, kept so an importer's own
    // program can see their symbols without re-emitting them.
    ParsedPhrases imported_context_phrases;
};

// prelude + everything transpiled separately so far — the symbol context for a
// unit whose imports live in their own .cpp files.
static ParsedPhrases symbolContext(const TranspileState& state) {
    ParsedPhrases context = state.prelude_phrases;
    context.insert(context.end(), state.imported_context_phrases.begin(), state.imported_context_phrases.end());
    return context;
}

static void writePreludeHeaderBeside(const std::string& cpp_path, const TranspileState& state) {
    const std::filesystem::path directory = directoryOf(cpp_path);
    writeTextFile((directory / "cprime-prelude.hpp").string(), state.prelude_header_code);
}

// Replaces `#include "<runtime header>"` lines with the embedded contents of that
// header, so a single-file build needs no runtime headers on disk. A header
// whose `#include` the emitter never produced (because nothing used it) is not
// inlined.
static std::string inlineRuntimeHeaders(std::string code) {
    const std::pair<std::string, std::string_view> runtime_headers[] = {
        {"#include \"c-prime.hpp\"", embedded::runtime_hpp()},
        {"#include \"io.hpp\"", embedded::io_hpp()},
        {"#include \"memory.hpp\"", embedded::memory_hpp()},
    };
    for (const auto& [directive, contents] : runtime_headers) {
        const size_t position = code.find(directive);
        if (position != std::string::npos) {
            code.replace(position, directive.size(), std::string(contents));
        }
    }
    return code;
}

// Every identifier / type token that appears anywhere under `phrase`.
static void collectReferencedNames(const std::shared_ptr<Phrase>& phrase, std::unordered_set<std::string>& names) {
    if (!phrase) {
        return;
    }
    for (const auto& token : phrase->tokens) {
        if (token
            && (token->type == TokenCatagory::Identifier || token->type == TokenCatagory::Type)
            && !token->value.empty()) {
            names.insert(token->value);
        }
    }
    for (const auto& nested : phrase->nested_phrases) {
        collectReferencedNames(nested, names);
    }
}

static std::unordered_set<std::string> collectReferencedNames(const ParsedPhrases& phrases) {
    std::unordered_set<std::string> names;
    for (const auto& phrase : phrases) {
        if (phrase) {
            collectReferencedNames(phrase->source_phrase, names);
        }
    }
    return names;
}

// The declared name of a top-level phrase, or "" when the phrase is not a named
// declaration (imports, trivia, bare statements) and so is always kept.
static std::string topLevelDeclarationName(const std::shared_ptr<ParsedPhrase>& phrase) {
    if (auto function_phrase = std::dynamic_pointer_cast<ParsedFunction>(phrase)) {
        return function_phrase->name;
    }
    if (auto type_phrase = std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase)) {
        return type_phrase->name;
    }
    if (auto struct_phrase = std::dynamic_pointer_cast<ParsedStructDefinition>(phrase)) {
        return struct_phrase->name;
    }
    if (auto enum_phrase = std::dynamic_pointer_cast<ParsedEnumDefinition>(phrase)) {
        return enum_phrase->name;
    }
    if (auto union_phrase = std::dynamic_pointer_cast<ParsedUnionDefinition>(phrase)) {
        return union_phrase->name;
    }
    if (auto variable_phrase = std::dynamic_pointer_cast<ParsedVariableDeclaration>(phrase)) {
        return variable_phrase->name;
    }
    return "";
}

// Drops the top-level declarations of an imported file that `used` never
// references, growing `used` to a fixed point so a kept declaration pulls in the
// other declarations it depends on.
static ParsedPhrases pruneUnusedDeclarations(const ParsedPhrases& phrases, std::unordered_set<std::string> used) {
    std::vector<char> keep(phrases.size(), 0);
    for (size_t i = 0; i < phrases.size(); ++i) {
        if (topLevelDeclarationName(phrases[i]).empty()) {
            keep[i] = 1;
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i < phrases.size(); ++i) {
            if (keep[i]) {
                continue;
            }
            if (used.find(topLevelDeclarationName(phrases[i])) != used.end()) {
                keep[i] = 1;
                changed = true;
                std::unordered_set<std::string> pulled;
                collectReferencedNames(phrases[i]->source_phrase, pulled);
                used.insert(pulled.begin(), pulled.end());
            }
        }
    }

    ParsedPhrases pruned;
    for (size_t i = 0; i < phrases.size(); ++i) {
        if (keep[i]) {
            pruned.push_back(phrases[i]);
        }
    }
    return pruned;
}

static ParsedPhrases parseUnitPhrases(const std::string& filename, TranspileState& state);
static std::string transpileCPrimeUnit(
    const std::string& source_path,
    TranspileState& state,
    const std::unordered_set<std::string>& used_by_importer
);

// Parses one file and resolves its imports. A native import becomes a normalised
// `#include`; a .cprime/.hprime import is either transpiled to its own .cpp and
// `#include`d (when `imports_dir` is set) or its phrases are inlined in place.
static ParsedPhrases parseUnitPhrases(const std::string& filename, TranspileState& state) {
    if (!fileExists(filename)) {
        throw std::runtime_error("Error: File " + filename + " does not exist.");
    }

    Tokens tokens = lexFile(filename);
    if (state.debug) {
        std::cout << "Found " << tokens.size() << " tokens in " << filename << ":\n";
        printTokens(tokens);
    }

    Phrases phrases = preparseTokens(tokens);
    if (state.debug) {
        std::cout << "Found " << phrases.size() << " phrases in " << filename << ":\n";
        printPhrases(phrases);
    }

    ParsedPhrases parsed_phrases = parsePhrases(phrases);
    const std::unordered_set<std::string> referenced = collectReferencedNames(parsed_phrases);

    ParsedPhrases expanded;
    for (const auto& parsed_phrase : parsed_phrases) {
        auto import_phrase = std::dynamic_pointer_cast<ParsedImportStatement>(parsed_phrase);
        if (!import_phrase) {
            expanded.push_back(parsed_phrase);
            continue;
        }
        if (import_phrase->path.empty()) {
            throw std::runtime_error("Import error: import path cannot be empty in " + filename);
        }

        const std::string resolved_path = resolvePathRelativeToFile(filename, import_phrase->path);
        if (!fileExists(resolved_path)) {
            throw std::runtime_error("Import error: File " + resolved_path + " does not exist.");
        }

        if (import_phrase->import_kind != ParsedImportKind::CPrime) {
            import_phrase->path = includePathForGeneratedCpp(resolved_path);
            expanded.push_back(import_phrase);
            continue;
        }

        if (!state.imports_dir.empty()) {
            const std::string generated_cpp = transpileCPrimeUnit(resolved_path, state, referenced);
            import_phrase->import_kind = ParsedImportKind::Cpp;
            import_phrase->path = includePathForGeneratedCpp(generated_cpp);
            expanded.push_back(import_phrase);
            continue;
        }

        // Inline the imported file's (optionally pruned) phrases in place.
        ParsedPhrases imported_phrases = parseUnitPhrases(resolved_path, state);
        if (!state.import_all) {
            imported_phrases = pruneUnusedDeclarations(imported_phrases, referenced);
        }
        if (import_phrase->source_phrase
            && !imported_phrases.empty()
            && imported_phrases.front()
            && imported_phrases.front()->source_phrase) {
            auto& destination = imported_phrases.front()->source_phrase->leading_trivia;
            const auto& carried = import_phrase->source_phrase->leading_trivia;
            destination.insert(destination.begin(), carried.begin(), carried.end());
        }
        expanded.insert(expanded.end(), imported_phrases.begin(), imported_phrases.end());
    }

    return expanded;
}

// Transpiles an imported .cprime/.hprime file into its own .cpp under
// `imports_dir` (memoised, cycle-safe). Returns the generated .cpp path.
static std::string transpileCPrimeUnit(
    const std::string& source_path,
    TranspileState& state,
    const std::unordered_set<std::string>& used_by_importer
) {
    const std::string key = includePathForGeneratedCpp(source_path);
    const auto done_it = state.transpiled.find(key);
    if (done_it != state.transpiled.end()) {
        return done_it->second;
    }

    const std::filesystem::path stem = std::filesystem::path(source_path).stem();
    const std::string output_cpp = (std::filesystem::path(state.imports_dir) / (stem.string() + ".cpp")).string();
    if (!state.in_progress.insert(key).second) {
        // Import cycle: the .cpp is still being written by an outer call, but its
        // path is already known, so a `#include` of it is enough.
        return output_cpp;
    }

    ParsedPhrases unit_phrases = parseUnitPhrases(source_path, state);
    if (!state.import_all) {
        unit_phrases = pruneUnusedDeclarations(unit_phrases, used_by_importer);
    }
    SemanticProgram program = buildSemanticProgramForUnit(symbolContext(state), unit_phrases);
    const CppEmitResult emitted = emitCpp(program);
    state.imported_context_phrases.insert(
        state.imported_context_phrases.end(), unit_phrases.begin(), unit_phrases.end());

    std::filesystem::create_directories(state.imports_dir);
    writeTextFile(output_cpp, "#pragma once\n#include \"cprime-prelude.hpp\"\n" + emitted.code);
    writeRuntimeHeadersBeside(output_cpp);
    writePreludeHeaderBeside(output_cpp, state);

    state.in_progress.erase(key);
    state.transpiled[key] = output_cpp;
    state.generated_paths.push_back(output_cpp);
    return output_cpp;
}

// The prelude is embedded in the binary and lexed straight from the embedded
// source. Any `import` of a runtime header it contains is dropped: the runtime
// headers (c-prime.hpp / std.hpp) are always emitted into the generated C++ and
// their symbols are always registered from the embedded copies, so the prelude
// does not need to pull them in explicitly.
static ParsedPhrases parseEmbeddedPrelude(bool debug) {
    Tokens tokens = lexSource(std::string(embedded::std_hprime()));
    Phrases phrases = preparseTokens(tokens);
    ParsedPhrases parsed = parsePhrases(phrases);

    ParsedPhrases without_imports{};
    for (const auto& phrase : parsed) {
        if (phrase && phrase->kind != ParsedPhraseKind::ImportStatement) {
            without_imports.push_back(phrase);
        }
    }

    if (debug) {
        std::cout << "Loaded " << without_imports.size() << " phrases from the embedded prelude\n";
    }
    return without_imports;
}

static int runCompiler(const CliOptions& options) {
    const std::string& filename = options.filename;
    std::cout << "C-Prime Compiler\n";

    if (!fileExists(filename)) {
        std::cout << "Error: File " << filename << " does not exist.\n";
        return EXIT_FAILURE;
    }

    try {
        TranspileState state{};
        state.debug = options.debug;
        state.imports_dir = options.imports_dir;
        state.import_all = options.import_all;
        state.prelude_phrases = parseEmbeddedPrelude(options.debug);
        for (const auto& prelude_phrase : state.prelude_phrases) {
            if (prelude_phrase) {
                clearPhraseTrivia(prelude_phrase->source_phrase);
            }
        }

        const bool separate_imports = !state.imports_dir.empty();

        // In separate mode the prelude is a shared header emitted whole, before
        // the imported units that `#include` it are transpiled.
        if (separate_imports) {
            state.prelude_header_code = "#pragma once\n"
                + emitCpp(buildSemanticProgram(state.prelude_phrases)).code;
        }

        // With `--imports <dir>` each imported .cprime file becomes its own .cpp
        // (already written by parseUnitPhrases); otherwise its phrases are
        // inlined here and the whole program, prelude included, is one unit.
        ParsedPhrases main_phrases = parseUnitPhrases(filename, state);
        if (options.debug) {
            printParsedPhrases(main_phrases);
        }

        SemanticProgram semantic_program;
        if (separate_imports) {
            semantic_program = buildSemanticProgramForUnit(symbolContext(state), main_phrases);
        } else {
            // Inline mode: the prelude is pruned like an imported file against
            // everything the fully-inlined program references.
            ParsedPhrases prelude = state.prelude_phrases;
            if (!state.import_all) {
                prelude = pruneUnusedDeclarations(prelude, collectReferencedNames(main_phrases));
            }
            prelude.insert(prelude.end(), main_phrases.begin(), main_phrases.end());
            semantic_program = buildSemanticProgram(prelude);
        }
        if (options.debug) {
            printSemanticSummary(semantic_program);
        }

        CppEmitResult emitted = emitCpp(semantic_program);
        const std::string output_filename = defaultCppOutputFilename(options);
        if (separate_imports) {
            // Imported units live in their own .cpp files; the runtime headers
            // are dropped beside the output so those `#include`s resolve.
            writeTextFile(output_filename, "#include \"cprime-prelude.hpp\"\n" + emitted.code);
            writeRuntimeHeadersBeside(output_filename);
            writePreludeHeaderBeside(output_filename, state);
        } else {
            // Single-file output: the runtime headers that are actually used are
            // inlined, so nothing extra is written to disk.
            writeTextFile(output_filename, inlineRuntimeHeaders(emitted.code));
        }

        if (shouldEmitCppFile(options)) {
            std::cout << "Generated C++ written to: " << output_filename << "\n";
            for (const auto& generated : state.generated_paths) {
                std::cout << "Generated C++ written to: " << generated << "\n";
            }
        }

        if (shouldCompileBinary(options)) {
            const std::string binary_filename = defaultBinaryFilename(options);
            const int compile_result = compileCppFile(output_filename, binary_filename);
            if (compile_result != 0) {
                std::cout << "Error: C++ compilation failed.\n";
                return EXIT_FAILURE;
            }

            std::cout << "Compiled binary written to: " << binary_filename << "\n";
        }

        if (options.mode == CliMode::CompileOnly) {
            removeFile(output_filename);
            for (const auto& generated : state.generated_paths) {
                removeFile(generated);
            }
        }

        if (options.debug) {
            std::cout << "Source map entries: " << emitted.source_map.size() << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return EXIT_FAILURE;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    const CliOptions options = parseCliOptions(argc, argv);

    if (options.help) {
        std::cout << usageText(argv[0]);
        return EXIT_SUCCESS;
    }

    if (!options.valid) {
        std::cout << usageText(argv[0]);
        return EXIT_FAILURE;
    }

    return runCompiler(options);
}
