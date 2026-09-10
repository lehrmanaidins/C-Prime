
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <variant>

#include "lexer/lexemizer/lexeme.hpp"

static const std::string RESET_COLOR = "\033[0m";
static const std::string ERROR_COLOR = "\033[31m";

void printError(const std::string& message, Lexeme lexeme) {
    static constexpr int INDENT_WIDTH = 6;
    static constexpr std::string_view SEPERATOR = " | ";

    LexemeType type = lexeme.type;
    std::string lexeme_text = lexeme.text;
    std::string filename = lexeme.filename;
    std::string line_text = lexeme.source_line_text;
    SourceLocation location = lexeme.location;

    const std::size_t indicator_position = location.column - 1;
    const std::size_t indicator_length = lexeme_text.size();
    const std::string preindicator_text = line_text.substr(0, indicator_position);
    const std::string postindicator_text = line_text.substr(indicator_position + lexeme_text.size());

    if (lexeme_text.empty()) {
        std::cout << filename << ":" << location.line << ":" << location.column << ": " << ERROR_COLOR << "error: " << RESET_COLOR << message << std::endl;
        return;
    }

    std::cout <<
        filename << ":" << location.line << ":" << location.column << ": " << ERROR_COLOR << "error: " << RESET_COLOR << message << "\n"
        << std::right << std::setw(INDENT_WIDTH) << std::to_string(location.line) << SEPERATOR << preindicator_text << ERROR_COLOR << lexeme_text << RESET_COLOR << postindicator_text << "\n"
        << std::right << std::string(INDENT_WIDTH, ' ') << SEPERATOR << std::string(indicator_position, ' ') << ERROR_COLOR << "^" << std::string(indicator_length - 1, '~') << RESET_COLOR << std::endl;
    ;
}

#include "lexer/tokenizer/token_variant.hpp"

void printError(const std::string& message, TokenVariant token) {
    printError(message, std::visit([](const auto& t) { return t.lexeme; }, token));
}
