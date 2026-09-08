
#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <unordered_map>

#include "io.cpp"

namespace lexemizer {

enum class LexemeType {
    WORD,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    CHARACTER_LITERAL,
    STRING_LITERAL,
    SYMBOL,
    WHITESPACE,
    COMMENT
};

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

struct Lexeme {
    LexemeType type;
    std::string text;
    std::size_t line;
    std::size_t column;
};

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

std::vector<Lexeme> lexemize(const std::vector<std::string>& source) {
    std::vector<Lexeme> lexemes;

    for (std::size_t line = 0; line < source.size(); ++line) {
        const std::string& line_str = source[line];

        std::size_t index = 0;
        std::size_t column = 1;

        std::cout << std::right
                  << std::setw(6)
                  << line + 1
                  << " | "
                  << line_str
                  << std::endl;

        while (index < line_str.size()) {
            std::optional<Lexeme> matched_lexeme;

            for (const LexemePattern& lexeme_pattern : lexeme_patterns) {
                std::smatch match;

                if (std::regex_search(
                        line_str.cbegin() + index,
                        line_str.cend(),
                        match,
                        lexeme_pattern.pattern,
                        std::regex_constants::match_continuous)) {

                    // Longest match wins.
                    if (!matched_lexeme ||
                        match.length() > matched_lexeme->text.length()) {

                        matched_lexeme = Lexeme{
                            lexeme_pattern.type,
                            match.str(),
                            line + 1,
                            column
                        };
                    }
                }
            }

            if (!matched_lexeme) {
                const unsigned char character =
                    static_cast<unsigned char>(line_str[index]);

                std::cerr
                    << "Unexpected character: '"
                    << line_str[index]
                    << "' (0x"
                    << std::hex
                    << static_cast<int>(character)
                    << std::dec
                    << ") at line "
                    << line + 1
                    << ", column "
                    << column
                    << std::endl;

                std::exit(EXIT_FAILURE);
            }

            lexemes.push_back(*matched_lexeme);

            index += matched_lexeme->text.size();
            column += matched_lexeme->text.size();
        }
    }

    return lexemes;
}

} // namespace lexemizer
