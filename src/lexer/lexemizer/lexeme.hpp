
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

static const std::unordered_map<LexemeType, std::string> lexeme_type_to_string {
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

struct Lexeme {
    LexemeType type;
    std::string lexeme_text;
    std::string filename;
    std::string line_text;
    std::size_t line_number;
    std::size_t column_number;

    std::string toString() const {
        std::string lexeme_str = "Lexeme = {";
        lexeme_str += lexeme_type_to_string.at(type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + filename + ", " + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
