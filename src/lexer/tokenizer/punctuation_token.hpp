
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class PunctuationType {
    Unknown,
    Comma,
    Semicolon
};

static const std::unordered_map<PunctuationType, std::string_view> punctuation_type_to_string = {
    {PunctuationType::Unknown, "Unknown"},
    {PunctuationType::Comma, "Comma"},
    {PunctuationType::Semicolon, "Semicolon"},
};

std::string_view toString(PunctuationType punctuation_type) {
    return punctuation_type_to_string.at(punctuation_type);
}

static const std::unordered_map<std::string, PunctuationType> punctuation_map = {
    {",", PunctuationType::Comma},
    {";", PunctuationType::Semicolon}
};

struct PunctuationToken : Token<TokenType::Punctuation> {
    PunctuationType punctuation_type;

    PunctuationToken(const Lexeme& lexeme)
        : Token<TokenType::Punctuation>(lexeme),
          punctuation_type(mapPunctuationType(lexeme))
    {}

    static PunctuationType mapPunctuationType(const Lexeme& lexeme) {
        auto it = punctuation_map.find(lexeme.lexeme_text);
        if (it != punctuation_map.end()) {
            return it->second;
        }
        throwLexerError("Unknown Punctuation: " + lexeme.lexeme_text, lexeme.line_text, lexeme.line_number, lexeme.column_number);
        return PunctuationType::Unknown;
    }

    static bool isPunctuation(const Lexeme& lexeme) {
        return punctuation_map.find(lexeme.lexeme_text) != punctuation_map.end();
    }
};

std::string toString(const PunctuationToken& token) {
    std::string lexeme_str = "PunctuationToken = {";
    lexeme_str += std::string(toString(token.punctuation_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
