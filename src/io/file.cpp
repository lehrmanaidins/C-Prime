#pragma once

#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

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
    const std::string command = std::string("c++ -std=c++26 ")
        + "-I . -I src "
        + shellQuote(cpp_filename)
        + " -o "
        + shellQuote(binary_filename);

    return std::system(command.c_str());
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