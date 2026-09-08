
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
 
#include "token.hpp"
#include "../lexer_error.hpp"

enum class WhiteSpaceType {
    Space,
    Tab,
    NewLine,
    CarriageReturn
};

static const std::unordered_map<std::string, WhiteSpaceType> whitespace_map = {
    {" ", WhiteSpaceType::Space},
    {"\t", WhiteSpaceType::Tab},
    {"\n", WhiteSpaceType::NewLine},
    {"\r", WhiteSpaceType::CarriageReturn}
};

struct WhiteSpaceToken : Token<TokenType::Whitespace> {
    WhiteSpaceType white_space_type;

    WhiteSpaceToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          white_space_type(mapWhiteSpaceType(lexeme, line, column))
    {}

    static WhiteSpaceType mapWhiteSpaceType(const std::string& lexeme, std::size_t line, std::size_t column) {
        auto it = whitespace_map.find(lexeme);
        if (it != whitespace_map.end()) {
            return it->second;
        }
        throwLexerError("Unknown WhiteSpace: " + lexeme, lexeme, line, column);
    }
};
