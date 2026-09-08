
#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <unordered_map>

#include "../../io.hpp"
#include "../lexer_error.hpp"
#include "lexeme.hpp"
#include "lexeme_pattern.hpp"

std::vector<Lexeme> lexemize(const std::vector<std::string>& source) {
    if (source.empty()) {
        std::cerr << "lexemize(): `source` is empty." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<Lexeme> lexemes;

    for (std::size_t line = 0; line < source.size(); ++line) {
        const std::string& line_str = source[line];

        std::size_t index = 0;
        std::size_t column = 1;

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
                    if (!matched_lexeme || match.length() > matched_lexeme->lexeme_text.length()) {
                        matched_lexeme = Lexeme{
                            .type = lexeme_pattern.type,
                            .lexeme_text = match.str(),
                            .line_text = line_str,
                            .line_number = line + 1,
                            .column_number = column
                        };
                    }
                }
            }

            if (!matched_lexeme) {
                const unsigned char character =
                    static_cast<unsigned char>(line_str[index]);

                throwLexerError("Unexpected character: " + std::string(1, line_str[index]), line_str, line + 1, column);
            }

            lexemes.push_back(*matched_lexeme);

            index += matched_lexeme->lexeme_text.size();
            column += matched_lexeme->lexeme_text.size();
        }
    }

    return lexemes;
}
