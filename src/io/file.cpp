#pragma once

#include <cstdlib>
#include <cstdio>
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
    const std::string command = "c++ -std=c++26 "
        + shellQuote(cpp_filename)
        + " -o "
        + shellQuote(binary_filename);

    return std::system(command.c_str());
}