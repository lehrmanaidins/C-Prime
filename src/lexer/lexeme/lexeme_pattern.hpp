
#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>
#include <regex>
#include <iostream>

#include "lexeme.hpp"

namespace lexemizer {
    
    inline constexpr std::string_view WORD_REGEX_PATTERN =
        R"([A-Za-z_][A-Za-z0-9_]*)";

    inline constexpr std::string_view INTEGER_LITERAL_REGEX_PATTERN =
        R"([0-9](?:_?[0-9])*(?:[eE][+]?[0-9](?:_?[0-9])*)?)";

    inline constexpr std::string_view FLOAT_LITERAL_REGEX_PATTERN =
        R"([0-9](?:_?[0-9])*\.[0-9](?:_?[0-9])*(?:[eE][+-]?[0-9](?:_?[0-9])*)?)";

    inline constexpr std::string_view STRING_LITERAL_REGEX_PATTERN =
        R"("(?:[^"\\]|\\.)*")";

    inline constexpr std::string_view CHARACTER_LITERAL_REGEX_PATTERN =
        R"('(?:[^'\\]|\\.)')";

    inline constexpr std::string_view SYMBOL_REGEX_PATTERN =
        R"([+\-*/%=<>!&|^~@#()[\]{};,.:?])";

    inline constexpr std::string_view WHITESPACE_REGEX_PATTERN =
        R"(\s+)";

    inline constexpr std::string_view COMMENT_REGEX_PATTERN =
        R"(//.*)";

    inline const std::unordered_map<LexemeType, std::string_view> LEXEME_TYPE_TO_STRING {
        {LexemeType::Word, "WORD"},
        {LexemeType::IntegerLiteral, "INTEGER_LITERAL"},
        {LexemeType::FloatLiteral, "FLOAT_LITERAL"},
        {LexemeType::CharacterLiteral, "CHARACTER_LITERAL"},
        {LexemeType::StringLiteral, "STRING_LITERAL"},
        {LexemeType::Symbol, "SYMBOL"},
        {LexemeType::Whitespace, "WHITESPACE"},
        {LexemeType::Comment, "COMMENT"}
    };

    std::ostream& operator<<(std::ostream& os, LexemeType type) {
        os << LEXEME_TYPE_TO_STRING.at(type);
        return os;
    }

    struct LexemePattern {
        LexemeType type;
        std::regex pattern;
    };

    inline const std::vector<LexemePattern> lexeme_patterns {
        {LexemeType::Word,              std::regex(std::string(WORD_REGEX_PATTERN))},
        {LexemeType::IntegerLiteral,   std::regex(std::string(INTEGER_LITERAL_REGEX_PATTERN))},
        {LexemeType::FloatLiteral,     std::regex(std::string(FLOAT_LITERAL_REGEX_PATTERN))},
        {LexemeType::StringLiteral,    std::regex(std::string(STRING_LITERAL_REGEX_PATTERN))},
        {LexemeType::CharacterLiteral, std::regex(std::string(CHARACTER_LITERAL_REGEX_PATTERN))},
        {LexemeType::Symbol,            std::regex(std::string(SYMBOL_REGEX_PATTERN))},
        {LexemeType::Whitespace,        std::regex(std::string(WHITESPACE_REGEX_PATTERN))},
        {LexemeType::Comment,           std::regex(std::string(COMMENT_REGEX_PATTERN))}
    };

} // namespace lexemizer