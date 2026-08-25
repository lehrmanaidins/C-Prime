
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <print>

#include "token.cpp"

Token getToken(std::ifstream& file) {
    std::string token = "";

    while (!file.eof()) {
        const char c = file.peek();
        if (!Token::isDelimiter(c)) {
            break;
        }
        file.get();
    }

    if (file.eof()) {
        return Token(token);
    }

    if (file.peek() == '/') {
        file.get();
        const bool isComment = !file.eof() && file.peek() == '/';
        file.unget();

        if (isComment) {
            std::string ignoredLine;
            std::getline(file, ignoredLine);
            return getToken(file);
        }
    }

    const char c = file.peek();
    if (Token::isSeparator(c)) {
        token += file.get();
        return Token(token);
    }

    const bool isNumberStart = std::isdigit(static_cast<unsigned char>(c)) ||
        (c == '.' && !file.eof() && [&file]() {
            const std::streampos pos = file.tellg();
            file.get();
            const char next = file.peek();
            file.seekg(pos);
            return std::isdigit(static_cast<unsigned char>(next));
        }());

    if (isNumberStart) {
        while (!file.eof()) {
            const char ch = file.peek();
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                token += file.get();
                continue;
            }

            if (ch == '.') {
                const std::streampos pos = file.tellg();
                file.get();
                const char next = file.peek();
                file.seekg(pos);
                if (std::isdigit(static_cast<unsigned char>(next))) {
                    token += file.get();
                    continue;
                }
            }

            break;
        }
        return Token(token);
    }

    if (Token::isOperatorStart(c)) {
        while (!file.eof()) {
            const char ch = file.peek();
            const std::string peeked = token + ch;

            if (!Token::isOperator(peeked)) {
                break;
            }

            token += file.get();
        }
        return Token(token);
    }

    if (c == '"' || c == '\'') {
        const char quote = file.get();
        token += quote;
        bool escaped = false;

        while (!file.eof()) {
            const char ch = file.get();
            token += ch;
            if (ch == quote && !escaped) {
                break;
            }
            escaped = ch == '\\' && !escaped;
            if (ch != '\\') {
                escaped = false;
            }
        }

        return Token(token);
    }

    while (!file.eof()) {
        const char ch = file.peek();

        if (Token::isDelimiter(ch) || Token::isSeparator(ch) || Token::isOperatorStart(ch)) {
            break;
        }

        token += file.get();
    }

    return Token(token);
}

Tokens getTokens(const std::string& filename) {
    Tokens tokens{};
    
    std::ifstream file(filename);

    while (true) {
        Token token = getToken(file);

        if (!token) {
            break;
        }

        tokens.push_back(std::make_shared<Token>(token));
    }

    return tokens;
}
    
Tokens lexFile(const std::string& filename) {
    Tokens tokens = getTokens(filename);

    return tokens;
}
