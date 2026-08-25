
#include <string>
#include <vector>
#include <fstream>
#include <print>

#include "token.cpp"

Token getToken(std::ifstream& file) {
    std::string token = "";

    // Skip leading delimiters
    while (!file.eof()) {
        char c = file.peek();
        // std::println("Skipping delimiter: {{}}", c);
        if (!Token::isDelimiter(c)) {
            break;
        }
        file.get();
    }

    if (!file.eof() && file.peek() == '/') {
        file.get();
        const bool isComment = !file.eof() && file.peek() == '/';
        file.unget();

        if (isComment) {
            std::string ignoredLine;
            std::getline(file, ignoredLine);
            return getToken(file);
        }
    }

    // Check if the next character is a separator and return it as a token if it is
    if (!file.eof()) {
        char c = file.peek();
        if (Token::isSeparator(c)) {
            token += file.get();
            return Token(token);
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

        return Token(token);
    }

    // Read token until delimiter or separator
    while (!file.eof()) {
        char c = file.peek();
        // std::println("Reading character: {{}}", c);

        if (Token::isDelimiter(c)) {
            break;
        }

        if (Token::isSeparator(c)) {
            break;
        }

        token += file.get();
    }

    return Token(token);
}

Tokens getTokens(const std::string& filename) {
    Tokens tokens{};
    std::string token = "";
    
    std::ifstream file(filename);
    while (!(token = getToken(file)).empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

Tokens lexFile(const std::string& filename) {
    Tokens tokens = getTokens(filename);

    return tokens;
}
