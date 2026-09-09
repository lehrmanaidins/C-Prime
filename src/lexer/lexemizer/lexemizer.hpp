
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

std::vector<Lexeme> sourceToLexemes(const std::vector<std::string>& source) {
    if (source.empty()) {
        std::cerr << "lexemize(): `source` is empty." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    static bool lexemizer_error = false;
    std::vector<Lexeme> lexemes;

    for (std::size_t line = 1; line < source.size(); line += 1) {
        const std::string& line_str = source[line - 1];

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
                            .line_number = line,
                            .column_number = column
                        };
                    }
                }
            }

            if (!matched_lexeme) {
                const unsigned char character =
                    static_cast<unsigned char>(line_str[index]);
                
                throwLexerError("Unexpected Character: " + std::string(1, line_str[index]), line_str, line, column);
                lexemizer_error = true;
                matched_lexeme = Lexeme{
                    .type = LexemeType::Unknown,
                    .lexeme_text = std::string(1, line_str[index]),
                    .line_text = line_str,
                    .line_number = line,
                    .column_number = column
                };
            }

            lexemes.push_back(*matched_lexeme);

            index += matched_lexeme->lexeme_text.size();
            column += matched_lexeme->lexeme_text.size();
        }
    }

    if (lexemizer_error) {
        std::exit(EXIT_FAILURE);
    }

    return lexemes;
}

bool isValidLexemeSequence(const Lexeme& current_lexeme, const Lexeme& next_lexeme) {
    const LexemeType current_type = current_lexeme.type;
    const LexemeType next_type = next_lexeme.type;

    const bool is_integer_followed_by_word = (current_type == LexemeType::IntegerLiteral && next_type == LexemeType::Word);
    const bool is_float_followed_by_word = (current_type == LexemeType::FloatLiteral && next_type == LexemeType::Word);

    const bool is_valid_lexeme_sequence = !(is_integer_followed_by_word || is_float_followed_by_word);

    return is_valid_lexeme_sequence;
}

std::vector<Lexeme> lexemize(const std::vector<std::string>& source) {
    std::vector<Lexeme> lexemes = sourceToLexemes(source);

    for (std::size_t i = 0; i + 1 < lexemes.size(); i += 1) {
        const Lexeme& current_lexeme = lexemes[i];
        const Lexeme& next_lexeme = lexemes[i + 1];

         if (!isValidLexemeSequence(current_lexeme, next_lexeme)) {
            throwLexerError(
                "Inproper Lexeme Sequence: " + current_lexeme.lexeme_text + next_lexeme.lexeme_text, 
                current_lexeme.line_text,
                current_lexeme.line_number,
                current_lexeme.column_number
            );
        }
    }

    return lexemes;
}
