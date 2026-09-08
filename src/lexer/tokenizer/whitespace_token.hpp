
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
 
#include "token.hpp"
#include "../lexer_error.hpp"

enum class WhiteSpaceType {
    Unknown,
    Space,
    Tab,
    NewLine,
    CarriageReturn
};

static const std::unordered_map<WhiteSpaceType, std::string_view> whitespace_type_to_string = {
    {WhiteSpaceType::Unknown, "Unknown"},
    {WhiteSpaceType::Space, "Space"},
    {WhiteSpaceType::Tab, "Tab"},
    {WhiteSpaceType::NewLine, "NewLine"},
    {WhiteSpaceType::CarriageReturn, "CarriageReturn"},
};

std::string_view toString(WhiteSpaceType white_space_type) {
    return whitespace_type_to_string.at(white_space_type);
}

static const std::unordered_map<std::string, WhiteSpaceType> whitespace_map = {
    {" ", WhiteSpaceType::Space},
    {"\t", WhiteSpaceType::Tab},
    {"\n", WhiteSpaceType::NewLine},
    {"\r", WhiteSpaceType::CarriageReturn}
};

struct WhiteSpaceToken : Token<TokenType::WhiteSpace> {
    WhiteSpaceType white_space_type;

    WhiteSpaceToken(const Lexeme& lexeme)
        : Token<TokenType::WhiteSpace>(lexeme),
          white_space_type(mapWhiteSpaceType(lexeme))
    {}

    static WhiteSpaceType mapWhiteSpaceType(const Lexeme& lexeme) {
        auto it = whitespace_map.find(lexeme.lexeme_text);
        if (it != whitespace_map.end()) {
            return it->second;
        }
        throwLexerError("Unknown WhiteSpace: " + lexeme.lexeme_text, lexeme.line_text, lexeme.line_number, lexeme.column_number);
        return WhiteSpaceType::Unknown;
    }

    static bool isWhiteSpace(const Lexeme& lexeme) {
        return whitespace_map.find(lexeme.lexeme_text) != whitespace_map.end();
    }
};

std::string toString(const WhiteSpaceToken& token) {
    std::string lexeme_str = "WhiteSpaceToken = {";
    lexeme_str += std::string(toString(token.white_space_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
