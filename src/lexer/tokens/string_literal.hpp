
#pragma once
 
#include "token.hpp"

enum class StringType {
    String
};

struct StringLiteralToken : Token<TokenType::StringLiteral> {
    StringType string_type;
};
