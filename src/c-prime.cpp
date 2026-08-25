
#include <string>
#include <vector>
#include <fstream>
#include <print>

#include "lexer/lexer.cpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::println("Usage: {} <filename>", argv[0]);
        return 1;
    }

    const std::string filename = argv[1];

    std::println("Lexing file: {}", filename);
    // Check if the file exists and can be opened
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println("Error: Could not open file {}", filename);
        return 1;
    }
    file.close();

    Tokens tokens = lexFile(filename);

    std::println("Found {} tokens:", tokens.size());

    for (const auto& token : tokens) {
        std::println("\ttoken type: {}\ttoken:\t{}", token->type, token->value);
    }

    return 0;
}
