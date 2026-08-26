
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

static std::string sourceBasename(const std::string& filename) {
    const std::string extension = ".cprime";
    if (filename.size() >= extension.size()
        && filename.substr(filename.size() - extension.size()) == extension) {
        return filename.substr(0, filename.size() - extension.size());
    }

    return filename;
}

static std::string defaultCppOutputFilename(const CliOptions& options) {
    if (!options.output_filename.empty()) {
        return options.output_filename;
    }

    return sourceBasename(options.filename) + ".cpp";
}

static std::string defaultBinaryFilename(const CliOptions& options) {
    if (!options.binary_filename.empty()) {
        return options.binary_filename;
    }

    return sourceBasename(options.filename);
}

static bool shouldEmitCppFile(const CliOptions& options) {
    return options.mode == CliMode::EmitCpp || options.mode == CliMode::EmitAndCompile;
}

static bool shouldCompileBinary(const CliOptions& options) {
    return options.mode == CliMode::CompileOnly || options.mode == CliMode::EmitAndCompile;
}

static std::string shellQuote(const std::string& value) {
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted += ch;
        }
    }
    quoted += "'";
    return quoted;
}

static int compileCppFile(const std::string& cpp_filename, const std::string& binary_filename) {
    const std::string command = "c++ -std=c++26 "
        + shellQuote(cpp_filename)
        + " -o "
        + shellQuote(binary_filename);

    return std::system(command.c_str());
}

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
