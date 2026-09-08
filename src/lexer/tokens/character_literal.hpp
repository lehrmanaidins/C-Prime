
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class CharacterType {
    Character
};

struct CharacterLiteralToken : Token<TokenType::CharacterLiteral> {
    CharacterType character_type;

    CharacterLiteralToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          character_type(CharacterType::Character)
    {}
};
