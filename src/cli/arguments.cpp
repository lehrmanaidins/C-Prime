#pragma once

#include <string>

enum class CliMode {
    EmitCpp,
    CompileOnly,
    EmitAndCompile
};

struct CliOptions {
    bool debug = false;
    bool help = false;
    bool valid = true;
    CliMode mode = CliMode::EmitCpp;
    std::string filename = "";
    std::string output_filename = "";
    std::string binary_filename = "";
    // `--imports <dir>`: transpile each imported .cprime/.hprime file into its
    // own .cpp under <dir> and `#include` it. Empty: inline imported code into
    // the main output file.
    std::string imports_dir = "";
    // `--import-all`: emit every top-level declaration of an imported file. By
    // default only the functions/types/values the program actually uses
    // (transitively) are transpiled.
    bool import_all = false;
};

static std::string usageText(const char* executable) {
    return "Usage: " + std::string(executable) + " [OPTIONS] <cprime-file>\n"
           "\n"
           "Modes:\n"
           "    --emit-cpp              Generate C++ only (default)\n"
           "    --compile               Compile to a binary only\n"
           "    --emit-and-compile      Generate C++ and compile a binary\n"
           "\n"
           "Output:\n"
           "    --output, -o <file>     C++ output filename (default: <cprime-file>.cpp)\n"
           "    --binary, -b <file>     Binary filename (default: <cprime-file without extension>)\n"
           "\n"
           "Imports:\n"
           "    --imports <dir>         Transpile imported .cprime/.hprime files into <dir>\n"
           "                            as their own .cpp files (default: inline them)\n"
           "    --import-all            Emit every declaration of imported files (default:\n"
           "                            only the used functions/types/values)\n"
           "\n"
           "Other:\n"
           "    --debug                 Print compiler pipeline debug output\n"
           "    --help, -h              Show this help text\n";
}

static CliOptions parseCliOptions(int argc, char* argv[]) {
    CliOptions options{};

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--debug") {
            options.debug = true;
            continue;
        }
        if (arg == "--help" || arg == "-h") {
            options.help = true;
            continue;
        }
        if (arg == "--emit-cpp") {
            options.mode = CliMode::EmitCpp;
            continue;
        }
        if (arg == "--compile") {
            options.mode = CliMode::CompileOnly;
            continue;
        }
        if (arg == "--emit-and-compile") {
            options.mode = CliMode::EmitAndCompile;
            continue;
        }
        if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) {
                options.output_filename = argv[i + 1];
                i++;
                continue;
            } else {
                options.valid = false;
                continue;
            }
        }
        if (arg == "--binary" || arg == "-b") {
            if (i + 1 < argc) {
                options.binary_filename = argv[i + 1];
                i++;
                continue;
            } else {
                options.valid = false;
                continue;
            }
        }
        if (arg == "--imports") {
            if (i + 1 < argc) {
                options.imports_dir = argv[i + 1];
                i++;
                continue;
            } else {
                options.valid = false;
                continue;
            }
        }
        if (arg == "--import-all") {
            options.import_all = true;
            continue;
        }

        if (options.filename.empty()) {
            options.filename = arg;
        } else {
            options.valid = false;
        }
    }

    if (!options.help && options.filename.empty()) {
        options.valid = false;
    }

    return options;
}

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