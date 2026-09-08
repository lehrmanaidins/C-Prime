
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class PunctuationType {
    Comma,
    Semicolon
};

static const std::unordered_map<std::string, PunctuationType> punctuation_map = {
    {",", PunctuationType::Comma},
    {";", PunctuationType::Semicolon}
};

struct PunctuationToken : Token<TokenType::Punctuation> {
    PunctuationType punctuation_type;

    PunctuationToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          punctuation_type(mapPunctuationType(lexeme, line, column))
    {}

    static PunctuationType mapPunctuationType(const std::string& lexeme, std::size_t line, std::size_t column) {
        auto it = punctuation_map.find(lexeme);
        if (it != punctuation_map.end()) {
            return it->second;
        }
        throwLexerError("Unknown Punctuation: " + lexeme, lexeme, line, column);
    }
};
