
#include <exception>
#include <cstdlib>
#include <iostream>
#include <string>

#include "lexer/lexer.cpp"
#include "parser/preparser.cpp"
#include "parser/parser.cpp"
#include "semantic/semantic.cpp"
#include "transpiler/cpp_emitter.cpp"
#include "cli/arguments.cpp"
#include "debug/print.cpp"
#include "io/file.cpp"

static int runCompiler(const CliOptions& options) {
    const std::string& filename = options.filename;
    std::cout << "C-Prime Compiler\n";

    if (!fileExists(filename)) {
        std::cout << "Error: File " << filename << " does not exist.\n";
        return EXIT_FAILURE;
    }

    Tokens tokens = lexFile(filename);

    if (options.debug) {
        printTokens(tokens);
    }

    Phrases phrases = preparseTokens(tokens);

    if (options.debug) {
        printPhrases(phrases);
    }

    try {
        ParsedPhrases parsed_phrases = parsePhrases(phrases);
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
