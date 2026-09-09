
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class KeywordTokenType {
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

static const std::unordered_map<KeywordTokenType, std::string> keyword_token_type_to_string = {
    {KeywordTokenType::Unknown, "Unknown"},
    {KeywordTokenType::Type, "Type"},
    {KeywordTokenType::Alias, "Alias"},
    {KeywordTokenType::Const, "Const"},
    {KeywordTokenType::Mutable, "Mutable"},
    {KeywordTokenType::Primitive, "Primitive"},
    {KeywordTokenType::Struct, "Struct"},
    {KeywordTokenType::Interface, "Interface"},
    {KeywordTokenType::Enum, "Enum"},
    {KeywordTokenType::Union, "Union"},
    {KeywordTokenType::Function, "Function"},
    {KeywordTokenType::Template, "Template"},
    {KeywordTokenType::Return, "Return"},
    {KeywordTokenType::Requires, "Requires"},
    {KeywordTokenType::Ensures, "Ensures"},
    {KeywordTokenType::If, "If"},
    {KeywordTokenType::Else, "Else"},
    {KeywordTokenType::Switch, "Switch"},
    {KeywordTokenType::Case, "Case"},
    {KeywordTokenType::For, "For"},
    {KeywordTokenType::ForEach, "ForEach"},
    {KeywordTokenType::Do, "Do"},
    {KeywordTokenType::While, "While"},
    {KeywordTokenType::Loop, "Loop"},
    {KeywordTokenType::Limit, "Limit"},
    {KeywordTokenType::Continue, "Continue"},
    {KeywordTokenType::Break, "Break"},
    {KeywordTokenType::True, "True"},
    {KeywordTokenType::False, "False"},
    {KeywordTokenType::Import, "Import"},
    {KeywordTokenType::Unsafe, "Unsafe"},
    {KeywordTokenType::Null, "Null"},
    {KeywordTokenType::Void, "Void"},
    {KeywordTokenType::Pure, "Pure"},
    {KeywordTokenType::Entry, "Entry"},
    {KeywordTokenType::Critical, "Critical"},
    {KeywordTokenType::Interrupt, "Interrupt"},
    {KeywordTokenType::Discard, "Discard"},
};

static const std::unordered_map<std::string, KeywordTokenType> keyword_map = {
    {"type", KeywordTokenType::Type},
    {"alias", KeywordTokenType::Alias},
    {"const", KeywordTokenType::Const},
    {"mutable", KeywordTokenType::Mutable},
    {"primitive", KeywordTokenType::Primitive},
    {"struct", KeywordTokenType::Struct},
    {"interface", KeywordTokenType::Interface},
    {"enum", KeywordTokenType::Enum},
    {"union", KeywordTokenType::Union},
    {"function", KeywordTokenType::Function},
    {"template", KeywordTokenType::Template},
    {"return", KeywordTokenType::Return},
    {"requires", KeywordTokenType::Requires},
    {"ensures", KeywordTokenType::Ensures},
    {"if", KeywordTokenType::If},
    {"else", KeywordTokenType::Else},
    {"switch", KeywordTokenType::Switch},
    {"case", KeywordTokenType::Case},
    {"for", KeywordTokenType::For},
    {"foreach", KeywordTokenType::ForEach},
    {"do", KeywordTokenType::Do},
    {"while", KeywordTokenType::While},
    {"loop", KeywordTokenType::Loop},
    {"limit", KeywordTokenType::Limit},
    {"continue", KeywordTokenType::Continue},
    {"break", KeywordTokenType::Break},
    {"true", KeywordTokenType::True},
    {"false", KeywordTokenType::False},
    {"import", KeywordTokenType::Import},
    {"unsafe", KeywordTokenType::Unsafe},
    {"null", KeywordTokenType::Null},
    {"void", KeywordTokenType::Void},
    {"pure", KeywordTokenType::Pure},
    {"entry", KeywordTokenType::Entry},
    {"critical", KeywordTokenType::Critical},
    {"interrupt", KeywordTokenType::Interrupt},
    {"discard", KeywordTokenType::Discard}
};

struct KeywordToken : Token {
    KeywordTokenType keyword_type;

    KeywordToken(const Lexeme& lexeme)
        : Token(lexeme),
         keyword_type(mapKeywordTokenType(lexeme))
    {}

    static KeywordTokenType mapKeywordTokenType(const Lexeme& lexeme) {
        auto it = keyword_map.find(lexeme.lexeme_text);
        if (it != keyword_map.end()) {
            return it->second;
        }
        
        printLexemeError("unknown keyword" + lexeme.lexeme_text, lexeme);
        return KeywordTokenType::Unknown;
    }

    static bool isKeyword(const Lexeme& lexeme) {
        return keyword_map.find(lexeme.lexeme_text) != keyword_map.end();
    }

    std::string toString() const override {
        std::string lexeme_str = "KeywordToken = {";
        lexeme_str += keyword_token_type_to_string.at(keyword_type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
