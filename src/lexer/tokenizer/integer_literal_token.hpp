
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../../error.hpp"

enum class IntegerLiteralTokenType {
    Unknown,
    Integer
};

static const std::unordered_map<IntegerLiteralTokenType, std::string> integer_literal_token_type_to_string = {
    {IntegerLiteralTokenType::Unknown, "Unknown"},
    {IntegerLiteralTokenType::Integer, "Integer"},
};

struct IntegerLiteralToken : Token {
    IntegerLiteralTokenType integer_literal_type;

    IntegerLiteralToken(const Lexeme& lexeme)
        : Token(lexeme),
          integer_literal_type(IntegerLiteralTokenType::Integer)
    {}

    std::string toString() const override {
        std::string lexeme_str = "IntegerLiteralToken = {";
        lexeme_str += integer_literal_token_type_to_string.at(integer_literal_type) + ", ";
        lexeme_str += "\"" + lexeme.text + "\", ";
        lexeme_str += "(" + std::to_string(lexeme.location.line) + ", ";
        lexeme_str += std::to_string(lexeme.location.column) + ")}";
        return lexeme_str;
    }
};
