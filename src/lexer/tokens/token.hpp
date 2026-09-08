
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

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

template <TokenType T>
struct Token {
    static constexpr TokenType token_type = T;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};
