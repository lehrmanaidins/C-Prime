
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>

void throwLexerError(const std::string& message, std::string code_line, std::size_t line, std::size_t column) {
    static constexpr std::string_view ERROR_PREFIX = "Lexer Error: ";
    static constexpr int INDENT_WIDTH = 6;
    static constexpr std::string_view SEPERATOR = " | ";

    const std::size_t code_line_text_start = code_line.find_first_not_of(" \t");
    if (code_line_text_start != std::string::npos) {
        code_line = code_line.substr(code_line_text_start);
    }

    std::cout <<
        ERROR_PREFIX << message << "\n"
        << std::right << std::setw(INDENT_WIDTH) << std::to_string(line) << SEPERATOR << code_line << "\n"
        << std::right << std::setw(INDENT_WIDTH + SEPERATOR.size() + column - code_line_text_start) << "^" << std::endl;
    ;
}
