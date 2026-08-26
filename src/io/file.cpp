#pragma once

#include <fstream>
#include <string>
#include <cstdio>

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