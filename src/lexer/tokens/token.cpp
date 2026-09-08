
#pragma once

#include <string>

enum class TokenType {
    Keyword,
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    CharacterLiteral,
    StringLiteral,
    Operator,
    Punctuation,
    Comment,
    Whitespace,
};

template <enum TokenType>
struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};
