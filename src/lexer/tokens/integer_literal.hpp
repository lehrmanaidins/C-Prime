
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class IntegerLiteralType {
    Integer
};

struct IntegerLiteralToken : Token<TokenType::IntegerLiteral> {
    IntegerLiteralType integer_literal_type;
};
