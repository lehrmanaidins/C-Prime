
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class KeywordType {
    Type,
    Alias,
    Const,
    Mutable,
    Primitive,
    Struct,
    Interface,
    Enum,
    Union,
    Function,
    Template,
    Return,
    Requires,
    Ensures,
    If,
    Else,
    Switch,
    Case,
    For,
    ForEach,
    Do,
    While,
    Loop,
    Limit,
    Continue,
    Break,
    True,
    False,
    Import,
    Unsafe,
    Null,
    Void,
    Pure,
    Entry,
    Critical,
    Interrupt,
    Discard
};

struct KeywordToken : Token<TokenType::Keyword> {
    KeywordType keyword_type;

    KeywordToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
         keyword_type(mapKeywordType(lexeme))
    {}

    static KeywordType mapKeywordType(const std::string& lexeme) {
        static const std::unordered_map<std::string, KeywordType> keyword_map = {
            {"type", KeywordType::Type},
            {"alias", KeywordType::Alias},
            {"const", KeywordType::Const},
            {"mutable", KeywordType::Mutable},
            {"primitive", KeywordType::Primitive},
            {"struct", KeywordType::Struct},
            {"interface", KeywordType::Interface},
            {"enum", KeywordType::Enum},
            {"union", KeywordType::Union},
            {"function", KeywordType::Function},
            {"template", KeywordType::Template},
            {"return", KeywordType::Return},
            {"requires", KeywordType::Requires},
            {"ensures", KeywordType::Ensures},
            {"if", KeywordType::If},
            {"else", KeywordType::Else},
            {"switch", KeywordType::Switch},
            {"case", KeywordType::Case},
            {"for", KeywordType::For},
            {"foreach", KeywordType::ForEach},
            {"do", KeywordType::Do},
            {"while", KeywordType::While},
            {"loop", KeywordType::Loop},
            {"limit", KeywordType::Limit},
            {"continue", KeywordType::Continue},
            {"break", KeywordType::Break},
            {"true", KeywordType::True},
            {"false", KeywordType::False},
            {"import", KeywordType::Import},
            {"unsafe", KeywordType::Unsafe},
            {"null", KeywordType::Null},
            {"void", KeywordType::Void},
            {"pure", KeywordType::Pure},
            {"entry", KeywordType::Entry},
            {"critical", KeywordType::Critical},
            {"interrupt", KeywordType::Interrupt},
            {"discard", KeywordType::Discard}
        };

        auto it = keyword_map.find(lexeme);
        if (it != keyword_map.end()) {
            return it->second;
        }
        
        throw std::runtime_error("Unknown keyword: " + lexeme);

    }
};
