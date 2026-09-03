#pragma once

#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "../embedded/resources.hpp"

static bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.is_open();
}

static void writeTextFile(const std::string& filename, const std::string& content) {
    std::ofstream output_file(filename);
    output_file << content;
}

static void removeFile(const std::string& filename) {
    std::remove(filename.c_str());
}

static std::string directoryOf(const std::string& filename) {
    const std::filesystem::path path(filename);
    return path.has_parent_path() ? path.parent_path().string() : ".";
}

// The generated C++ includes "c-prime.hpp" / "std.hpp" by basename. Those
// headers are embedded in the transpiler; drop a copy next to the emitted source
// so the generated code compiles without the C-Prime source tree present.
static void writeRuntimeHeadersBeside(const std::string& output_filename) {
    const std::filesystem::path directory = directoryOf(output_filename);
    writeTextFile((directory / "c-prime.hpp").string(), std::string(embedded::runtime_hpp()));
    writeTextFile((directory / "std.hpp").string(), std::string(embedded::std_hpp()));
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
    const std::string quoted_cpp = shellQuote(cpp_filename);
    const std::string includes = "-I " + shellQuote(directoryOf(cpp_filename)) + " -I . -I src";

    // 1. Format the generated source in place (picks up the repo .clang-format).
    std::system(("clang-format -i " + quoted_cpp).c_str());

    // 2. Compile with clang++ using an aggressive warning set, warnings as
    //    errors, so that any sloppy code the transpiler emits fails the build.
    const std::string warnings =
        "-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow "
        "-Wold-style-cast -Wcast-qual -Wdouble-promotion -Wformat=2 "
        "-Wnull-dereference -Wimplicit-fallthrough -Wextra-semi -Wunused "
        "-Wnon-virtual-dtor -Woverloaded-virtual -Werror";
    const std::string compile_command = "clang++ -std=c++26 " + includes + " " + warnings + " "
        + quoted_cpp + " -o " + shellQuote(binary_filename);
    const int compile_status = std::system(compile_command.c_str());
    if (compile_status != 0) {
        return compile_status;
    }

    // 3. Static analysis with clang-tidy (repo .clang-tidy), findings are errors.
    const std::string tidy_command = "clang-tidy --quiet --warnings-as-errors=* "
        + quoted_cpp + " -- -std=c++26 " + includes;
    return std::system(tidy_command.c_str());
}

static std::string resolvePathRelativeToFile(const std::string& base_filename, const std::string& path) {
    const std::filesystem::path import_path(path);
    if (import_path.is_absolute()) {
        return import_path.lexically_normal().string();
    }

    const std::filesystem::path base_path(base_filename);
    const std::filesystem::path base_dir = base_path.has_parent_path()
        ? base_path.parent_path()
        : std::filesystem::current_path();

    return (base_dir / import_path).lexically_normal().string();
}

static std::string includePathForGeneratedCpp(const std::string& path) {
    std::filesystem::path import_path(path);
    if (import_path.is_absolute()) {
        std::error_code error;
        std::filesystem::path relative_path = std::filesystem::relative(import_path, std::filesystem::current_path(), error);
        if (!error) {
            import_path = relative_path;
        }
    }

    return import_path.lexically_normal().generic_string();
}