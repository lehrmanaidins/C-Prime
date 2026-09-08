
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class FloatLiteralType {
    Float
};

struct FloatLiteralToken : Token<TokenType::FloatLiteral> {
    FloatLiteralType float_literal_type;

    FloatLiteralToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          float_literal_type(FloatLiteralType::Float)
    {}
};
