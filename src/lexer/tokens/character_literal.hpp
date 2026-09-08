
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
};
