
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class OperatorTokenType {
    Unknown,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Percent,
    Equals,
    LessThan,
    GreaterThan,
    Exclamation,
    Ampersand,
    Pipe,
    Caret,
    Tilde,
    Dot,
    Colon,
    Question,
    LeftParenthesis,
    RightParenthesis,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    At,
    Octothorpe,
    Dollar
};

static const std::unordered_map<OperatorTokenType, std::string> operator_token_type_to_string = {
    {OperatorTokenType::Unknown, "Unknown"},
    {OperatorTokenType::Plus, "Plus"},
    {OperatorTokenType::Minus, "Minus"},
    {OperatorTokenType::Asterisk, "Asterisk"},
    {OperatorTokenType::Slash, "Slash"},
    {OperatorTokenType::Percent, "Percent"},
    {OperatorTokenType::Equals, "Equals"},
    {OperatorTokenType::LessThan, "LessThan"},
    {OperatorTokenType::GreaterThan, "GreaterThan"},
    {OperatorTokenType::Exclamation, "Exclamation"},
    {OperatorTokenType::Ampersand, "Ampersand"},
    {OperatorTokenType::Pipe, "Pipe"},
    {OperatorTokenType::Caret, "Caret"},
    {OperatorTokenType::Tilde, "Tilde"},
    {OperatorTokenType::Dot, "Dot"},
    {OperatorTokenType::Colon, "Colon"},
    {OperatorTokenType::Question, "Question"},
    {OperatorTokenType::LeftParenthesis, "LeftParenthesis"},
    {OperatorTokenType::RightParenthesis, "RightParenthesis"},
    {OperatorTokenType::LeftBracket, "LeftBracket"},
    {OperatorTokenType::RightBracket, "RightBracket"},
    {OperatorTokenType::LeftBrace, "LeftBrace"},
    {OperatorTokenType::RightBrace, "RightBrace"},
    {OperatorTokenType::At, "At"},
    {OperatorTokenType::Octothorpe, "Octothorpe"},
    {OperatorTokenType::Dollar, "Dollar"},
};

static const std::unordered_map<std::string, OperatorTokenType> operator_map = {
    {"+", OperatorTokenType::Plus},
    {"-", OperatorTokenType::Minus},
    {"*", OperatorTokenType::Asterisk},
    {"/", OperatorTokenType::Slash},
    {"%", OperatorTokenType::Percent},
    {"=", OperatorTokenType::Equals},
    {"<", OperatorTokenType::LessThan},
    {">", OperatorTokenType::GreaterThan},
    {"!", OperatorTokenType::Exclamation},
    {"&", OperatorTokenType::Ampersand},
    {"|", OperatorTokenType::Pipe},
    {"^", OperatorTokenType::Caret},
    {"~", OperatorTokenType::Tilde},
    {".", OperatorTokenType::Dot},
    {":", OperatorTokenType::Colon},
    {"?", OperatorTokenType::Question},
    {"(", OperatorTokenType::LeftParenthesis},
    {")", OperatorTokenType::RightParenthesis},
    {"[", OperatorTokenType::LeftBracket},
    {"]", OperatorTokenType::RightBracket},
    {"{", OperatorTokenType::LeftBrace},
    {"}", OperatorTokenType::RightBrace},
    {"@", OperatorTokenType::At},
    {"#", OperatorTokenType::Octothorpe},
    {"$", OperatorTokenType::Dollar}
};

struct OperatorToken : Token {
    OperatorTokenType operator_type;

    OperatorToken(const Lexeme& lexeme)
        : Token(lexeme),
          operator_type(mapOperatorTokenType(lexeme))
    {}

    static OperatorTokenType mapOperatorTokenType(const Lexeme& lexeme) {
        auto it = operator_map.find(lexeme.lexeme_text);
        if (it != operator_map.end()) {
            return it->second;
        }
        
        throwLexerError("Unknown Operator: " + lexeme.lexeme_text, lexeme.line_text, lexeme.line_number, lexeme.column_number);
        return OperatorTokenType::Unknown;
    }

    static bool isOperator(const Lexeme& lexeme) {
        return operator_map.find(lexeme.lexeme_text) != operator_map.end();
    }

    std::string toString() const override {
        std::string lexeme_str = "OperatorToken = {";
        lexeme_str += operator_token_type_to_string.at(operator_type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
