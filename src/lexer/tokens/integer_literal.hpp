
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class IntegerLiteralType {
    Integer
};

struct IntegerLiteralToken : Token<TokenType::IntegerLiteral> {
    IntegerLiteralType integer_literal_type;

    IntegerLiteralToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          integer_literal_type(IntegerLiteralType::Integer)
    {}
};
