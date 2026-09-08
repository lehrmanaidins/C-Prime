
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class FloatLiteralType {
    Unknown,
    Float
};

static const std::unordered_map<FloatLiteralType, std::string_view> float_literal_type_to_string = {
    {FloatLiteralType::Unknown, "Unknown"},
    {FloatLiteralType::Float, "Float"},
};

std::string_view toString(FloatLiteralType float_literal_type) {
    return float_literal_type_to_string.at(float_literal_type);
}

struct FloatLiteralToken : Token<TokenType::FloatLiteral> {
    FloatLiteralType float_literal_type;

    FloatLiteralToken(const Lexeme& lexeme)
        : Token<TokenType::FloatLiteral>(lexeme),
          float_literal_type(FloatLiteralType::Float)
    {}
};

std::string toString(const FloatLiteralToken& token) {
    std::string lexeme_str = "FloatLiteralToken = {";
    lexeme_str += std::string(toString(token.float_literal_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
