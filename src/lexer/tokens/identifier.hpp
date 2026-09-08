
#pragma once
 
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
