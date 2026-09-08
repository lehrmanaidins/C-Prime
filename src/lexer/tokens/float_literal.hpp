
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class FloatLiteralType {
    Float
};

struct FloatLiteralToken : Token<TokenType::FloatLiteral> {
    FloatLiteralType float_literal_type;
};
