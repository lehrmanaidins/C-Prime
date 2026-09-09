
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class StringTokenType {
    Unknown,
    String
};

static const std::unordered_map<StringTokenType, std::string> string_token_type_to_string = {
    {StringTokenType::Unknown, "Unknown"},
    {StringTokenType::String, "String"},
};

struct StringLiteralToken : Token {
    StringTokenType string_type;

    StringLiteralToken(const Lexeme& lexeme)
        : Token(lexeme),
          string_type(StringTokenType::String)
    {}

    std::string toString() const override {
        std::string lexeme_str = "StringLiteralToken = {";
        lexeme_str += string_token_type_to_string.at(string_type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
