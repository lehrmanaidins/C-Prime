
#pragma once

#include <string>
#include <unordered_map>
#include <cstddef>

enum class LexemeType {
    Unknown,
    Word,
    IntegerLiteral,
    FloatLiteral,
    CharacterLiteral,
    StringLiteral,
    Symbol,
    Whitespace,
    Comment
};

static const std::unordered_map<LexemeType, std::string> lexeme_string_map {
    {LexemeType::Unknown, "Unknown"},
    {LexemeType::Word, "Word"},
    {LexemeType::IntegerLiteral, "IntegerLiteral"},
    {LexemeType::FloatLiteral, "FloatLiteral"},
    {LexemeType::CharacterLiteral, "CharacterLiteral"},
    {LexemeType::StringLiteral, "StringLiteral"},
    {LexemeType::Symbol, "Symbol"},
    {LexemeType::Whitespace, "Whitespace"},
    {LexemeType::Comment, "Comment"}
};

std::string toString(LexemeType type) {
    return lexeme_string_map.at(type);
}

struct Lexeme {
    LexemeType type;
    std::string lexeme_text;
    std::string line_text;
    std::size_t line_number;
    std::size_t column_number;
};

std::string toString(const Lexeme& lexeme) {
    std::string lexeme_str = "Lexeme = {";
    lexeme_str += toString(lexeme.type) + ", ";
    lexeme_str += "\"" + lexeme.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(lexeme.line_number) + ", ";
    lexeme_str += std::to_string(lexeme.column_number) + ")}";
    return lexeme_str;
}
