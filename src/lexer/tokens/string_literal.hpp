
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class StringType {
    String
};

struct StringLiteralToken : Token<TokenType::StringLiteral> {
    StringType string_type;
};
