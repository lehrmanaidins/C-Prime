
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class OperatorType {
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

    OperatorToken(std::string lexeme, std::size_t line, std::size_t column, OperatorType operator_type)
        : Token(lexeme, line, column),
          operator_type(mapOperatorType(lexeme))
    {}

    static OperatorType mapOperatorType(const std::string& lexeme) {
        auto it = operator_map.find(lexeme);
        if (it != operator_map.end()) {
            return it->second;
        }
        throw std::invalid_argument("Invalid operator lexeme: " + lexeme);
    }

    static bool isOperator(const std::string& lexeme) {
        return operator_map.find(lexeme) != operator_map.end();
    }
};
