
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
 
#include "token.hpp"
#include "../lexer_error.hpp"

enum class WhiteSpaceTokenType {
    Unknown,
    Space,
    Tab,
    NewLine,
    CarriageReturn
};

static const std::unordered_map<WhiteSpaceTokenType, std::string> whitespace_token_type_to_string = {
    {WhiteSpaceTokenType::Unknown, "Unknown"},
    {WhiteSpaceTokenType::Space, "Space"},
    {WhiteSpaceTokenType::Tab, "Tab"},
    {WhiteSpaceTokenType::NewLine, "NewLine"},
    {WhiteSpaceTokenType::CarriageReturn, "CarriageReturn"},
};

static const std::unordered_map<std::string, WhiteSpaceTokenType> whitespace_map = {
    {" ", WhiteSpaceTokenType::Space},
    {"\t", WhiteSpaceTokenType::Tab},
    {"\n", WhiteSpaceTokenType::NewLine},
    {"\r", WhiteSpaceTokenType::CarriageReturn}
};

struct WhiteSpaceToken : Token {
    WhiteSpaceTokenType white_space_type;

    WhiteSpaceToken(const Lexeme& lexeme)
        : Token(lexeme),
          white_space_type(mapWhiteSpaceType(lexeme))
    {}

    static WhiteSpaceTokenType mapWhiteSpaceType(const Lexeme& lexeme) {
        auto it = whitespace_map.find(lexeme.lexeme_text);
        if (it != whitespace_map.end()) {
            return it->second;
        }
        printLexemeError("unknown whitespace" + lexeme.lexeme_text, lexeme);
        return WhiteSpaceTokenType::Unknown;
    }

    static bool isWhiteSpace(const Lexeme& lexeme) {
        return whitespace_map.find(lexeme.lexeme_text) != whitespace_map.end();
    }

    std::string toString() const override {
        std::string lexeme_str = "WhiteSpaceToken = {";
        lexeme_str += whitespace_token_type_to_string.at(white_space_type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
