
#pragma once

#include <string>
#include <unordered_map>
#include <cstddef>

#include "../../source/source_location.hpp"

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
    std::string text;
    std::string filename;
    std::string source_line_text;
    SourceLocation location;

    std::string toString() const {
        std::string lexeme_str = "Lexeme = {";
        lexeme_str += lexeme_type_to_string.at(type) + ", ";
        lexeme_str += "\"" + text + "\", ";
        lexeme_str += "(" + filename + ", " + std::to_string(location.line) + ", ";
        lexeme_str += std::to_string(location.column) + ", ";
        lexeme_str += "\"" + source_line_text + "\")";
        lexeme_str += "}";
        return lexeme_str;
    }
};
