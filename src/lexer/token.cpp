
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#if __has_include(<format>)
#include <format>
#endif

using Terms = std::vector<std::string>;

enum class TokenCatagory {
    Default,
    Separator,
    Keyword,
    Primitive,
    Type,
    Identifier,
    Literal,
    Operator,
    Unknown
};

#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
template <>
struct std::formatter<TokenCatagory> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const TokenCatagory& type, std::format_context& ctx) const {
        std::string_view name;
        switch (type) {
            case TokenCatagory::Default: name = "Default"; break;
            case TokenCatagory::Separator: name = "Separator"; break;
            case TokenCatagory::Keyword: name = "Keyword"; break;
            case TokenCatagory::Primitive: name = "Primitive"; break;
            case TokenCatagory::Type: name = "Type"; break;
            case TokenCatagory::Identifier: name = "Identifier"; break;
            case TokenCatagory::Literal: name = "Literal"; break;
            case TokenCatagory::Operator: name = "Operator"; break;
            case TokenCatagory::Unknown: name = "Unknown"; break;
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};
#endif

std::string decodeEscapedToken(const std::string& value) {
    std::string decoded;
    decoded.reserve(value.size());

    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[i + 1];
            if (next == 'n') {
                decoded.push_back('\n');
                ++i;
                continue;
            }
            if (next == 't') {
                decoded.push_back('\t');
                ++i;
                continue;
            }
            if (next == 'r') {
                decoded.push_back('\r');
                ++i;
                continue;
            }
            if (next == '\\') {
                decoded.push_back('\\');
                ++i;
                continue;
            }
        }
        decoded.push_back(value[i]);
    }

    return decoded;
}

Terms loadTerms(const std::string& filename, bool emptyLineAsSpace = false) {
    std::ifstream termsFile(filename);
    std::vector<std::string> terms{};
    std::string line = "";

    while (std::getline(termsFile, line)) {
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            if (emptyLineAsSpace) {
                terms.push_back(" ");
            }
            continue;
        }

        line = line.substr(start);

        if (line[0] == '/' || line[0] == '#') {
            continue;
        }

        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        size_t end = line.find_last_not_of(" \t");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        }

        if (line.empty()) {
            if (emptyLineAsSpace) {
                terms.push_back(" ");
            }
            continue;
        }

        terms.push_back(decodeEscapedToken(line));
    }

    return terms;
}

struct Token {
    const std::string value;
    TokenCatagory type;
    size_t line;
    size_t column;

    Token(const std::string& val, size_t line = 0, size_t column = 0)
        : value(val), type(TokenCatagory::Default), line(line), column(column) {
        findTokenType();
    }

    operator std::string() const { return value; }

    operator bool() const { return !value.empty(); }

    static bool isSeparator(char c) {
        static const Terms separators = loadTerms("src/lists/separators.txt");
        return std::find(separators.begin(), separators.end(), std::string(1, c)) != separators.end();
    }

    static bool isDelimiter(char c) {
        static const Terms delimiters = loadTerms("src/lists/delimiters.txt", true);
        return std::find(delimiters.begin(), delimiters.end(), std::string(1, c)) != delimiters.end();
    }

    static bool isKeyword(const std::string& str) {
        static const Terms keywords = loadTerms("src/lists/keywords.txt");
        return std::find(keywords.begin(), keywords.end(), str) != keywords.end();
    }

    static bool isPrimitive(const std::string& str) {
        static const Terms primitives = loadTerms("src/lists/primitives.txt");
        return std::find(primitives.begin(), primitives.end(), str) != primitives.end();
    }

    static bool isLiteral(const std::string& str) {
        if (str.size() >= 2 &&
            ((str.front() == '"' && str.back() == '"') ||
            (str.front() == '\'' && str.back() == '\''))) {
            return true;
        }

        bool hasDecimalPoint = false;
        bool hasDigit = false;
        if (str.empty()) {
            return false;
        }

        for (char c : str) {
            if (std::isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;
                continue;
            }
            if (c == '.' && !hasDecimalPoint) {
                hasDecimalPoint = true;
                continue;
            }
            return false;
        }

        return hasDigit;
    }

    static bool isOperator(const std::string& str) {
        static const Terms operators = loadTerms("src/lists/operators.txt");
        return std::find(operators.begin(), operators.end(), str) != operators.end();
    }

    static bool isOperatorStart(char c) {
        static const Terms operators = loadTerms("src/lists/operators.txt");
        for (const auto& op : operators) {
            if (!op.empty() && op.front() == c) {
                return true;
            }
        }
        return false;
    }

    static bool isIdentifier(const std::string& str) {
        if (str.empty()) {
            return false;
        }

        for (const char& c : str) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                return false;
            }
        }

        return true;
    }

    private:

    void findTokenType() {
        if (value.empty()) {
            return;
        }
       
        if (isSeparator(value[0])) {
            type = TokenCatagory::Separator;
            return;
        }

        if (isKeyword(value)) {
            type = TokenCatagory::Keyword;
            return;
        }

        if (isPrimitive(value)) {
            type = TokenCatagory::Primitive;
            return;
        }

        if (isLiteral(value)) {
            type = TokenCatagory::Literal;
            return;
        }

        if (isOperator(value)) {
            type = TokenCatagory::Operator;
            return;
        }

        if (isIdentifier(value)) {
            type = TokenCatagory::Identifier;
            return;
        }

        type = TokenCatagory::Unknown;
    }
};

using Tokens = std::vector<std::shared_ptr<Token>>;
