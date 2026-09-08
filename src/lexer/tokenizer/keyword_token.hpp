
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class KeywordType {
    Unknown,
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

static const std::unordered_map<KeywordType, std::string_view> keyword_type_to_string = {
    {KeywordType::Unknown, "Unknown"},
    {KeywordType::Type, "Type"},
    {KeywordType::Alias, "Alias"},
    {KeywordType::Const, "Const"},
    {KeywordType::Mutable, "Mutable"},
    {KeywordType::Primitive, "Primitive"},
    {KeywordType::Struct, "Struct"},
    {KeywordType::Interface, "Interface"},
    {KeywordType::Enum, "Enum"},
    {KeywordType::Union, "Union"},
    {KeywordType::Function, "Function"},
    {KeywordType::Template, "Template"},
    {KeywordType::Return, "Return"},
    {KeywordType::Requires, "Requires"},
    {KeywordType::Ensures, "Ensures"},
    {KeywordType::If, "If"},
    {KeywordType::Else, "Else"},
    {KeywordType::Switch, "Switch"},
    {KeywordType::Case, "Case"},
    {KeywordType::For, "For"},
    {KeywordType::ForEach, "ForEach"},
    {KeywordType::Do, "Do"},
    {KeywordType::While, "While"},
    {KeywordType::Loop, "Loop"},
    {KeywordType::Limit, "Limit"},
    {KeywordType::Continue, "Continue"},
    {KeywordType::Break, "Break"},
    {KeywordType::True, "True"},
    {KeywordType::False, "False"},
    {KeywordType::Import, "Import"},
    {KeywordType::Unsafe, "Unsafe"},
    {KeywordType::Null, "Null"},
    {KeywordType::Void, "Void"},
    {KeywordType::Pure, "Pure"},
    {KeywordType::Entry, "Entry"},
    {KeywordType::Critical, "Critical"},
    {KeywordType::Interrupt, "Interrupt"},
    {KeywordType::Discard, "Discard"},
};

std::string_view toString(KeywordType keyword_type) {
    return keyword_type_to_string.at(keyword_type);
}

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

struct KeywordToken : Token<TokenType::Keyword> {
    KeywordType keyword_type;

    KeywordToken(const Lexeme& lexeme)
        : Token<TokenType::Keyword>(lexeme),
         keyword_type(mapKeywordType(lexeme))
    {}

    static KeywordType mapKeywordType(const Lexeme& lexeme) {
        auto it = keyword_map.find(lexeme.lexeme_text);
        if (it != keyword_map.end()) {
            return it->second;
        }
        
        throwLexerError("Unknown Keyword: " + lexeme.lexeme_text, lexeme.line_text, lexeme.line_number, lexeme.column_number);
        return KeywordType::Unknown;
    }

    static bool isKeyword(const Lexeme& lexeme) {
        return keyword_map.find(lexeme.lexeme_text) != keyword_map.end();
    }
};

std::string toString(const KeywordToken& token) {
    std::string lexeme_str = "KeywordToken = {";
    lexeme_str += std::string(toString(token.keyword_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
