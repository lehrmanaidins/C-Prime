
#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

namespace io {

std::ifstream openFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file \'" << filename << "\'" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return file;
}

void closeFile(std::ifstream& file) {
    if (file.is_open()) {
        file.close();
    }
}

std::vector<std::string> readFileLines(const std::string& filename) {
    std::ifstream file = openFile(filename);
    
    std::vector<std::string> lines{};
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    closeFile(file);
    return lines;
}

} // namespace io
