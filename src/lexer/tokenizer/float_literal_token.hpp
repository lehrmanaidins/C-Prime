
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../../error.hpp"

enum class FloatLiteralTokenType {
    Unknown,
    Float
};

static const std::unordered_map<FloatLiteralTokenType, std::string> float_literal_token_type_to_string = {
    {FloatLiteralTokenType::Unknown, "Unknown"},
    {FloatLiteralTokenType::Float, "Float"},
};

struct FloatLiteralToken : Token {
    FloatLiteralTokenType float_literal_type;

    FloatLiteralToken(const Lexeme& lexeme)
        : Token(lexeme),
          float_literal_type(FloatLiteralTokenType::Float)
    {}

    std::string toString() const override {
        std::string lexeme_str = "FloatLiteralToken = {";
        lexeme_str += float_literal_token_type_to_string.at(float_literal_type) + ", ";
        lexeme_str += "\"" + lexeme.text + "\", ";
        lexeme_str += "(" + std::to_string(lexeme.location.line) + ", ";
        lexeme_str += std::to_string(lexeme.location.column) + ")}";
        return lexeme_str;
    }
};

