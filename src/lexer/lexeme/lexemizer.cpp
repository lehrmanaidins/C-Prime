
#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <unordered_map>

#include "../../io.cpp"
#include "lexeme.cpp"
#include "lexeme_pattern.cpp"

namespace lexemizer {

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
