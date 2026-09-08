
#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>
#include <regex>
#include <iostream>

#include "lexeme.cpp"

namespace lexemizer {
    
    inline constexpr std::string_view WORD_REGEX_PATTERN =
        R"([A-Za-z_][A-Za-z0-9_]*)";

    inline constexpr std::string_view INTEGER_LITERAL_REGEX_PATTERN =
        R"([0-9]+)";

    inline constexpr std::string_view FLOAT_LITERAL_REGEX_PATTERN =
        R"([0-9]+\.[0-9]+)";

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
        {LexemeType::WORD, "WORD"},
        {LexemeType::INTEGER_LITERAL, "INTEGER_LITERAL"},
        {LexemeType::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {LexemeType::CHARACTER_LITERAL, "CHARACTER_LITERAL"},
        {LexemeType::STRING_LITERAL, "STRING_LITERAL"},
        {LexemeType::SYMBOL, "SYMBOL"},
        {LexemeType::WHITESPACE, "WHITESPACE"},
        {LexemeType::COMMENT, "COMMENT"}
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
        {LexemeType::WORD,              std::regex(std::string(WORD_REGEX_PATTERN))},
        {LexemeType::INTEGER_LITERAL,   std::regex(std::string(INTEGER_LITERAL_REGEX_PATTERN))},
        {LexemeType::FLOAT_LITERAL,     std::regex(std::string(FLOAT_LITERAL_REGEX_PATTERN))},
        {LexemeType::STRING_LITERAL,    std::regex(std::string(STRING_LITERAL_REGEX_PATTERN))},
        {LexemeType::CHARACTER_LITERAL, std::regex(std::string(CHARACTER_LITERAL_REGEX_PATTERN))},
        {LexemeType::SYMBOL,            std::regex(std::string(SYMBOL_REGEX_PATTERN))},
        {LexemeType::WHITESPACE,        std::regex(std::string(WHITESPACE_REGEX_PATTERN))},
        {LexemeType::COMMENT,           std::regex(std::string(COMMENT_REGEX_PATTERN))}
    };

} // namespace lexemizer