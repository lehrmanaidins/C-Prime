
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "lexemizer/lexeme.hpp"
#include "tokenizer/token.hpp"

static const std::string RESET_COLOR = "\033[0m";
static const std::string ERROR_COLOR = "\033[31m";

void printLexemeError(const std::string& message, Lexeme lexeme) {
    static constexpr int INDENT_WIDTH = 6;
    static constexpr std::string_view SEPERATOR = " | ";

    LexemeType type = lexeme.type;
    std::string lexeme_text = lexeme.lexeme_text;
    std::string filename = lexeme.filename;
    std::string line_text = lexeme.line_text;
    std::size_t line_number = lexeme.line_number;
    std::size_t column_number = lexeme.column_number; 

    const std::size_t indicator_position = column_number - 1;
    const std::size_t indicator_length = lexeme_text.size();
    const std::string preindicator_text = line_text.substr(0, indicator_position);
    const std::string postindicator_text = line_text.substr(indicator_position + lexeme_text.size());

    if (lexeme_text.empty()) {
        std::cout << filename << ":" << line_number << ":" << column_number << ": " << ERROR_COLOR << "error: " << RESET_COLOR << message << std::endl;
        return;
    }

    std::cout <<
        filename << ":" << line_number << ":" << column_number << ": " << ERROR_COLOR << "error: " << RESET_COLOR << message << "\n"
        << std::right << std::setw(INDENT_WIDTH) << std::to_string(line_number) << SEPERATOR << preindicator_text << ERROR_COLOR << lexeme_text << RESET_COLOR << postindicator_text << "\n"
        << std::right << std::string(INDENT_WIDTH, ' ') << SEPERATOR << std::string(indicator_position, ' ') << ERROR_COLOR << "^" << std::string(indicator_length - 1, '~') << RESET_COLOR << std::endl;
    ;
}
