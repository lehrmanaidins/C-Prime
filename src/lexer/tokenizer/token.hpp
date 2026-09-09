
#pragma once

#include <string>
#include <unordered_map>

#include "../lexemizer/lexeme.hpp"

struct Token {
    std::string lexeme_text;
    std::string line_text;
    std::size_t line_number;
    std::size_t column_number;

    Token(const Lexeme& lexeme)
        : lexeme_text(lexeme.lexeme_text),
          line_text(lexeme.line_text),
          line_number(lexeme.line_number),
          column_number(lexeme.column_number)
    {}

    virtual ~Token() = default;

    virtual std::string toString() const = 0;
};
