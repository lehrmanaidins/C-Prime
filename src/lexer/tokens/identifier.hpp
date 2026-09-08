
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class IdentifierType {
    BuiltInPrimitiveType,
    BuiltInOtherType,
    BuiltInFunctionName,
    UserDefined,
};

struct IdentifierToken : Token<TokenType::Identifier> {
    IdentifierType identifier_type;
};
