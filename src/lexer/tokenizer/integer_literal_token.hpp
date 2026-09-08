
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class IntegerLiteralType {
    Unknown,
    Integer
};

static const std::unordered_map<IntegerLiteralType, std::string_view> integer_literal_type_to_string = {
    {IntegerLiteralType::Unknown, "Unknown"},
    {IntegerLiteralType::Integer, "Integer"},
};

std::string_view toString(IntegerLiteralType integer_literal_type) {
    return integer_literal_type_to_string.at(integer_literal_type);
}

struct IntegerLiteralToken : Token<TokenType::IntegerLiteral> {
    IntegerLiteralType integer_literal_type;

    IntegerLiteralToken(const Lexeme& lexeme)
        : Token<TokenType::IntegerLiteral>(lexeme),
          integer_literal_type(IntegerLiteralType::Integer)
    {}
};

std::string toString(const IntegerLiteralToken& token) {
    std::string lexeme_str = "IntegerLiteralToken = {";
    lexeme_str += std::string(toString(token.integer_literal_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
