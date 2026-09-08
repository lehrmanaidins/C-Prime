
#pragma once
 
#include "token.hpp"

enum class CharacterType {
    Character
};

struct CharacterLiteralToken : Token<TokenType::CharacterLiteral> {
    CharacterType character_type;
};
