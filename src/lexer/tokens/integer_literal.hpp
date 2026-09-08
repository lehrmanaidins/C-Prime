
#pragma once
 
#include "token.hpp"

enum class IntegerLiteralType {
    Integer
};

struct IntegerLiteralToken : Token<TokenType::IntegerLiteral> {
    IntegerLiteralType integer_literal_type;
};
