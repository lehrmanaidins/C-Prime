
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

struct Source {
    std::string filename;
    std::vector<std::string> lines;
};

std::vector<Lexeme> sourceToLexemes(const Source& source) {
    if (source.lines.empty()) {
        printLexemeError("source is empty", Lexeme{
            .type = LexemeType::Unknown,
            .lexeme_text = "",
            .filename = source.filename,
            .line_text = "",
            .line_number = 0,
            .column_number = 0
        });
        std::exit(EXIT_FAILURE);
    }

    static bool lexemizer_error = false;
    std::vector<Lexeme> lexemes;

    for (std::size_t line = 1; line < source.lines.size(); line += 1) {
        const std::string& line_str = source.lines[line - 1];

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
                            .filename = source.filename,
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
                
                printLexemeError("invalid character", Lexeme{
                    .type = LexemeType::Unknown,
                    .lexeme_text = std::string(1, line_str[index]),
                    .filename = source.filename,
                    .line_text = line_str,
                    .line_number = line,
                    .column_number = column
                });
                lexemizer_error = true;
                matched_lexeme = Lexeme{
                    .type = LexemeType::Unknown,
                    .lexeme_text = std::string(1, line_str[index]),
                    .filename = source.filename,
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

void checkForValidLexemeSequences(const std::string& filename, const std::vector<Lexeme>& lexemes) {
    static bool lexeme_sequence_error = false;
    for (std::size_t i = 0; i + 1 < lexemes.size(); i += 1) {
        const Lexeme& current_lexeme = lexemes[i];
        const Lexeme& next_lexeme = lexemes[i + 1];

         if (!isValidLexemeSequence(current_lexeme, next_lexeme)) {
            lexeme_sequence_error = true;
            printLexemeError(
                "invalid lexeme",
                Lexeme{
                    .type = current_lexeme.type,
                    .lexeme_text = current_lexeme.lexeme_text + next_lexeme.lexeme_text,
                    .filename = current_lexeme.filename,
                    .line_text = current_lexeme.line_text,
                    .line_number = current_lexeme.line_number,
                    .column_number = current_lexeme.column_number
                }
            );
        }
    }
    if (lexeme_sequence_error) {
        std::exit(EXIT_FAILURE);
    }
}

std::vector<Lexeme> lexemize(const Source& source) {
    std::vector<Lexeme> lexemes = sourceToLexemes(source);
    checkForValidLexemeSequences(source.filename, lexemes);
    return lexemes;
}
