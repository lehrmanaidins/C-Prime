
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>

void throwLexerError(const std::string& message, const std::string& code_line, std::size_t line, std::size_t column) {
    static constexpr std::string_view ERROR_PREFIX = "Lexer Error: ";
    static constexpr int INDENT_WIDTH = 8;
    static constexpr std::string_view SEPERATOR = " | ";

    std::cout <<
        ERROR_PREFIX << message << "\n"
        << std::right << std::setw(INDENT_WIDTH) << std::to_string(line) << SEPERATOR << code_line << "\n"
        << std::right << std::setw(INDENT_WIDTH + SEPERATOR.size() + code_line.size()) << "^" << std::endl;
    ;

    std::exit(EXIT_FAILURE);
}
