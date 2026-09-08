
#pragma once
 
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

struct OperatorToken : Token<TokenType::Operator> {
    OperatorType operator_type;
};
