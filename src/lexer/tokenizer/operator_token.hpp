
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class OperatorType {
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

static const std::unordered_map<OperatorType, std::string_view> operator_type_to_string = {
    {OperatorType::Unknown, "Unknown"},
    {OperatorType::Plus, "Plus"},
    {OperatorType::Minus, "Minus"},
    {OperatorType::Asterisk, "Asterisk"},
    {OperatorType::Slash, "Slash"},
    {OperatorType::Percent, "Percent"},
    {OperatorType::Equals, "Equals"},
    {OperatorType::LessThan, "LessThan"},
    {OperatorType::GreaterThan, "GreaterThan"},
    {OperatorType::Exclamation, "Exclamation"},
    {OperatorType::Ampersand, "Ampersand"},
    {OperatorType::Pipe, "Pipe"},
    {OperatorType::Caret, "Caret"},
    {OperatorType::Tilde, "Tilde"},
    {OperatorType::Dot, "Dot"},
    {OperatorType::Colon, "Colon"},
    {OperatorType::Question, "Question"},
    {OperatorType::LeftParenthesis, "LeftParenthesis"},
    {OperatorType::RightParenthesis, "RightParenthesis"},
    {OperatorType::LeftBracket, "LeftBracket"},
    {OperatorType::RightBracket, "RightBracket"},
    {OperatorType::LeftBrace, "LeftBrace"},
    {OperatorType::RightBrace, "RightBrace"},
    {OperatorType::At, "At"},
    {OperatorType::Octothorpe, "Octothorpe"},
    {OperatorType::Dollar, "Dollar"},
};

std::string_view toString(OperatorType operator_type) {
    return operator_type_to_string.at(operator_type);
}

static const std::unordered_map<std::string, OperatorType> operator_map = {
    {"+", OperatorType::Plus},
    {"-", OperatorType::Minus},
    {"*", OperatorType::Asterisk},
    {"/", OperatorType::Slash},
    {"%", OperatorType::Percent},
    {"=", OperatorType::Equals},
    {"<", OperatorType::LessThan},
    {">", OperatorType::GreaterThan},
    {"!", OperatorType::Exclamation},
    {"&", OperatorType::Ampersand},
    {"|", OperatorType::Pipe},
    {"^", OperatorType::Caret},
    {"~", OperatorType::Tilde},
    {".", OperatorType::Dot},
    {":", OperatorType::Colon},
    {"?", OperatorType::Question},
    {"(", OperatorType::LeftParenthesis},
    {")", OperatorType::RightParenthesis},
    {"[", OperatorType::LeftBracket},
    {"]", OperatorType::RightBracket},
    {"{", OperatorType::LeftBrace},
    {"}", OperatorType::RightBrace},
    {"@", OperatorType::At},
    {"#", OperatorType::Octothorpe},
    {"$", OperatorType::Dollar}
};

struct OperatorToken : Token<TokenType::Operator> {
    OperatorType operator_type;

    OperatorToken(const Lexeme& lexeme)
        : Token<TokenType::Operator>(lexeme),
          operator_type(mapOperatorType(lexeme))
    {}

    static OperatorType mapOperatorType(const Lexeme& lexeme) {
        auto it = operator_map.find(lexeme.lexeme_text);
        if (it != operator_map.end()) {
            return it->second;
        }
        
        throwLexerError("Unknown Operator: " + lexeme.lexeme_text, lexeme.line_text, lexeme.line_number, lexeme.column_number);
        return OperatorType::Unknown;
    }

    static bool isOperator(const Lexeme& lexeme) {
        return operator_map.find(lexeme.lexeme_text) != operator_map.end();
    }
};

std::string toString(const OperatorToken& token) {
    std::string lexeme_str = "OperatorToken = {";
    lexeme_str += std::string(toString(token.operator_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
