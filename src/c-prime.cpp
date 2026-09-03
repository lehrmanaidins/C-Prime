
#include <exception>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_set>

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

static ParsedPhrases parseCPrimeFileWithImports(
    const std::string& filename,
    bool debug,
    std::unordered_set<std::string>& importing
) {
    if (importing.find(filename) != importing.end()) {
        return {};
    }

    if (!fileExists(filename)) {
        throw std::runtime_error("Error: File " + filename + " does not exist.");
    }

    importing.insert(filename);

    Tokens tokens = lexFile(filename);

    if (debug) {
        std::cout << "Found " << tokens.size() << " tokens in " << filename << ":\n";
        printTokens(tokens);
    }

    Phrases phrases = preparseTokens(tokens);

    if (debug) {
        std::cout << "Found " << phrases.size() << " phrases in " << filename << ":\n";
        printPhrases(phrases);
    }

    ParsedPhrases parsed_phrases = parsePhrases(phrases);
    ParsedPhrases expanded{};

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

        if (import_phrase->import_kind == ParsedImportKind::CPrime) {
            ParsedPhrases imported_phrases = parseCPrimeFileWithImports(resolved_path, debug, importing);
            if (import_phrase->source_phrase
                && !imported_phrases.empty()
                && imported_phrases.front()
                && imported_phrases.front()->source_phrase) {
                auto& destination = imported_phrases.front()->source_phrase->leading_trivia;
                const auto& carried = import_phrase->source_phrase->leading_trivia;
                destination.insert(destination.begin(), carried.begin(), carried.end());
            }
            expanded.insert(expanded.end(), imported_phrases.begin(), imported_phrases.end());
            continue;
        }

        import_phrase->path = includePathForGeneratedCpp(resolved_path);
        expanded.push_back(import_phrase);
    }

    importing.erase(filename);
    return expanded;
}

static ParsedPhrases parseCPrimeFileWithImports(const std::string& filename, bool debug) {
    std::unordered_set<std::string> importing{};
    return parseCPrimeFileWithImports(filename, debug, importing);
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
        std::unordered_set<std::string> importing{};
        ParsedPhrases parsed_phrases = parseEmbeddedPrelude(options.debug);
        for (const auto& prelude_phrase : parsed_phrases) {
            if (prelude_phrase) {
                clearPhraseTrivia(prelude_phrase->source_phrase);
            }
        }
        ParsedPhrases source_phrases = parseCPrimeFileWithImports(filename, options.debug, importing);
        parsed_phrases.insert(parsed_phrases.end(), source_phrases.begin(), source_phrases.end());
        if (options.debug) {
            printParsedPhrases(parsed_phrases);
        }

        SemanticProgram semantic_program = buildSemanticProgram(parsed_phrases);

        if (options.debug) {
            printSemanticSummary(semantic_program);
        }

        CppEmitResult emitted = emitCpp(semantic_program);
        const std::string output_filename = defaultCppOutputFilename(options);
        writeTextFile(output_filename, emitted.code);
        writeRuntimeHeadersBeside(output_filename);

        if (shouldEmitCppFile(options)) {
            std::cout << "Generated C++ written to: " << output_filename << "\n";
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
