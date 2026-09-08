
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "../lexemizer/lexeme.hpp"
#include "token.hpp"
#include "../lexer_error.hpp"

enum class CharacterType {
    Unknown,
    Character
};

static const std::unordered_map<CharacterType, std::string_view> character_type_to_string = {
    {CharacterType::Unknown, "Unknown"},
    {CharacterType::Character, "Character"},
};

std::string_view toString(CharacterType character_type) {
    return character_type_to_string.at(character_type);
}

struct CharacterLiteralToken : Token<TokenType::CharacterLiteral> {
    CharacterType character_type;

    CharacterLiteralToken(const Lexeme& lexeme)
        : Token<TokenType::CharacterLiteral>(lexeme),
          character_type(CharacterType::Character)
    {}
};

std::string toString(const CharacterLiteralToken& token) {
    std::string lexeme_str = "CharacterLiteralToken = {";
    lexeme_str += std::string(toString(token.character_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
