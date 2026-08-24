
#include <string>
#include <vector>
#include <fstream>
#include <print>

#include "token.cpp"

namespace lexer {

Token getToken(std::ifstream& file) {
    std::string token = "";

    // Skip leading delimiters
    while (!file.eof()) {
        char c = file.peek();
        // std::println("Skipping delimiter: {{}}", c);
        if (!isDelimiter(c)) {
            break;
        }
        file.get();
    }

    // Check if the next character is a separator and return it as a token if it is
    if (!file.eof()) {
        char c = file.peek();
        if (isSeparator(c)) {
            token += file.get();
            return token;
        }
    }

    if (!file.eof() && (file.peek() == '"' || file.peek() == '\'')) {
        const char quote = file.get();
        token += quote;
        bool escaped = false;

        while (!file.eof()) {
            const char c = file.get();
            token += c;
            if (c == quote && !escaped) {
                break;
            }
            escaped = c == '\\' && !escaped;
            if (c != '\\') {
                escaped = false;
            }
        }

        return token;
    }

    // Read token until delimiter or separator
    while (!file.eof()) {
        char c = file.peek();
        // std::println("Reading character: {{}}", c);

        if (isDelimiter(c)) {
            break;
        }

        if (isSeparator(c)) {
            break;
        }

        token += file.get();
    }

    return Token(token);
}

Tokens lexFile(const std::string& filename) {
    std::ifstream file(filename);

    Tokens tokens{};
    std::string token = "";

    while (!(token = getToken(file)).empty()) {
        // std::println("token: {}", token);
        tokens.push_back(token);
    }

    return tokens;
}

}

/*
 * Lexer for C-Prime source files.
 * @param filename The name of the C-Prime source file to lex.
 */
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

    lexer::Tokens tokens = lexer::lexFile(filename);

    std::println("Found {} tokens:", tokens.size());

    for (const lexer::Token& token : tokens) {
        std::println("\ttoken type: {}\ttoken:\t\"{}\"", token.type, token.value);
    }

    return 0;
}
