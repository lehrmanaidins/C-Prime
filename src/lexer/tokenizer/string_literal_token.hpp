
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class StringType {
    Unknown,
    String
};

static const std::unordered_map<StringType, std::string_view> string_type_to_string = {
    {StringType::Unknown, "Unknown"},
    {StringType::String, "String"},
};

std::string_view toString(StringType string_type) {
    return string_type_to_string.at(string_type);
}

struct StringLiteralToken : Token<TokenType::StringLiteral> {
    StringType string_type;

    StringLiteralToken(const Lexeme& lexeme)
        : Token<TokenType::StringLiteral>(lexeme),
          string_type(StringType::String)
    {}
};

std::string toString(const StringLiteralToken& token) {
    std::string lexeme_str = "StringLiteralToken = {";
    lexeme_str += std::string(toString(token.string_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
