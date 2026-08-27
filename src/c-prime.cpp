
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

static int runCompiler(const CliOptions& options) {
    const std::string& filename = options.filename;
    std::cout << "C-Prime Compiler\n";

    if (!fileExists(filename)) {
        std::cout << "Error: File " << filename << " does not exist.\n";
        return EXIT_FAILURE;
    }

    try {
        ParsedPhrases parsed_phrases = parseCPrimeFileWithImports(filename, options.debug);
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
