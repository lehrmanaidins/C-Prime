
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class PunctuationTokenType {
    Unknown,
    Comma,
    Semicolon
};

static const std::unordered_map<PunctuationTokenType, std::string> punctuation_token_type_to_string = {
    {PunctuationTokenType::Unknown, "Unknown"},
    {PunctuationTokenType::Comma, "Comma"},
    {PunctuationTokenType::Semicolon, "Semicolon"},
};

static const std::unordered_map<std::string, PunctuationTokenType> punctuation_map = {
    {",", PunctuationTokenType::Comma},
    {";", PunctuationTokenType::Semicolon}
};

struct PunctuationToken : Token {
    PunctuationTokenType punctuation_type;

    PunctuationToken(const Lexeme& lexeme)
        : Token(lexeme),
          punctuation_type(mapPunctuationTokenType(lexeme))
    {}

    static PunctuationTokenType mapPunctuationTokenType(const Lexeme& lexeme) {
        auto it = punctuation_map.find(lexeme.lexeme_text);
        if (it != punctuation_map.end()) {
            return it->second;
        }
        printLexemeError("unknown punctuation" + lexeme.lexeme_text, lexeme);
        return PunctuationTokenType::Unknown;
    }

    static bool isPunctuation(const Lexeme& lexeme) {
        return punctuation_map.find(lexeme.lexeme_text) != punctuation_map.end();
    }

    std::string toString() const override {
        std::string lexeme_str = "PunctuationToken = {";
        lexeme_str += punctuation_token_type_to_string.at(punctuation_type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
