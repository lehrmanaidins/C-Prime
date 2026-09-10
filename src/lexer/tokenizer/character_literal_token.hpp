
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "../lexemizer/lexeme.hpp"
#include "token.hpp"
#include "../../error.hpp"

enum class CharacterLiteralTokenType {
    Unknown,
    Character
};

static const std::unordered_map<CharacterLiteralTokenType, std::string> character_literal_token_type_to_string = {
    {CharacterLiteralTokenType::Unknown, "Unknown"},
    {CharacterLiteralTokenType::Character, "Character"},
};

struct CharacterLiteralToken : Token {
    CharacterLiteralTokenType character_type;

    CharacterLiteralToken(const Lexeme& lexeme)
        : Token(lexeme),
          character_type(CharacterLiteralTokenType::Character)
    {}

    std::string toString() const override {
        std::string lexeme_str = "CharacterLiteralToken = {";
        lexeme_str += character_literal_token_type_to_string.at(character_type) + ", ";
        lexeme_str += "\"" + lexeme.text + "\", ";
        lexeme_str += "(" + std::to_string(lexeme.location.line) + ", ";
        lexeme_str += std::to_string(lexeme.location.column) + ")}";
        return lexeme_str;
    }
};
