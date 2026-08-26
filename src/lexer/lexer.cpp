#pragma once

#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "token.cpp"

struct SourceCursor {
    explicit SourceCursor(const std::string& source)
        : source(source), index(0), line(1), column(1) {
    }

    const std::string& source;
    size_t index;
    size_t line;
    size_t column;

    bool eof() const {
        return index >= source.size();
    }

    char peek(size_t offset = 0) const {
        if (index + offset >= source.size()) {
            return '\0';
        }

        return source[index + offset];
    }

    char get() {
        if (eof()) {
            return '\0';
        }

        const char ch = source[index++];
        if (ch == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }

        return ch;
    }
};

static void skipDelimiters(SourceCursor& cursor) {
    while (!cursor.eof() && Token::isDelimiter(cursor.peek())) {
        cursor.get();
    }
}

static void skipCommentIfPresent(SourceCursor& cursor, bool& skipped_comment) {
    skipped_comment = false;

    if (cursor.peek() != '/') {
        return;
    }

    if (cursor.peek(1) == '/') {
        cursor.get();
        cursor.get();

        while (!cursor.eof() && cursor.peek() != '\n') {
            cursor.get();
        }

        skipped_comment = true;
        return;
    }

    if (cursor.peek(1) == '*') {
        cursor.get();
        cursor.get();

        while (!cursor.eof()) {
            const char ch = cursor.get();
            if (ch == '*' && cursor.peek() == '/') {
                cursor.get();
                break;
            }
        }

        skipped_comment = true;
    }
}

Token getToken(SourceCursor& cursor) {
    std::string token;

    while (true) {
        skipDelimiters(cursor);

        bool skipped_comment = false;
        skipCommentIfPresent(cursor, skipped_comment);
        if (!skipped_comment) {
            break;
        }
    }

    if (cursor.eof()) {
        return Token(token, cursor.line, cursor.column);
    }

    const size_t token_line = cursor.line;
    const size_t token_column = cursor.column;

    const char c = cursor.peek();
    if (Token::isSeparator(c)) {
        token += cursor.get();
        return Token(token, token_line, token_column);
    }

    const bool is_number_start = std::isdigit(static_cast<unsigned char>(c))
        || (c == '.' && std::isdigit(static_cast<unsigned char>(cursor.peek(1))));

    if (is_number_start) {
        while (!cursor.eof()) {
            const char ch = cursor.peek();
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                token += cursor.get();
                continue;
            }

            if (ch == '.' && std::isdigit(static_cast<unsigned char>(cursor.peek(1)))) {
                token += cursor.get();
                continue;
            }

            break;
        }

        return Token(token, token_line, token_column);
    }

    if (Token::isOperatorStart(c)) {
        while (!cursor.eof()) {
            const char ch = cursor.peek();
            const std::string next_value = token + ch;
            if (!Token::isOperator(next_value)) {
                break;
            }

            token += cursor.get();
        }

        return Token(token, token_line, token_column);
    }

    if (c == '"' || c == '\'') {
        const char quote = cursor.get();
        token += quote;

        bool escaped = false;
        while (!cursor.eof()) {
            const char ch = cursor.get();
            token += ch;

            if (ch == quote && !escaped) {
                break;
            }

            if (ch == '\\' && !escaped) {
                escaped = true;
            } else {
                escaped = false;
            }
        }

        return Token(token, token_line, token_column);
    }

    while (!cursor.eof()) {
        const char ch = cursor.peek();
        if (Token::isDelimiter(ch) || Token::isSeparator(ch) || Token::isOperatorStart(ch)) {
            break;
        }

        token += cursor.get();
    }

    return Token(token, token_line, token_column);
}

Tokens getTokens(const std::string& filename) {
    Tokens tokens{};

    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string source = buffer.str();

    SourceCursor cursor(source);

    while (true) {
        Token token = getToken(cursor);
        if (!token) {
            break;
        }

        tokens.push_back(std::make_shared<Token>(token));
    }

    return tokens;
}

Tokens lexFile(const std::string& filename) {
    return getTokens(filename);
}
