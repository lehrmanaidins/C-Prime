
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

static const std::unordered_map<std::string, IdentifierType> identifier_map = {
    {"uint8", IdentifierType::BuiltInPrimitiveType},
    {"uint16", IdentifierType::BuiltInPrimitiveType},
    {"uint32", IdentifierType::BuiltInPrimitiveType},
    {"uint64", IdentifierType::BuiltInPrimitiveType},
    {"int8", IdentifierType::BuiltInPrimitiveType},
    {"int16", IdentifierType::BuiltInPrimitiveType},
    {"int32", IdentifierType::BuiltInPrimitiveType},
    {"int64", IdentifierType::BuiltInPrimitiveType},
    {"float32", IdentifierType::BuiltInPrimitiveType},
    {"float64", IdentifierType::BuiltInPrimitiveType},
    {"char8", IdentifierType::BuiltInPrimitiveType},
    {"char16", IdentifierType::BuiltInPrimitiveType},
    {"char32", IdentifierType::BuiltInPrimitiveType},
    {"bool", IdentifierType::BuiltInPrimitiveType},

    {"void", IdentifierType::BuiltInOtherType},
    {"pointer", IdentifierType::BuiltInOtherType},
    {"reference", IdentifierType::BuiltInOtherType},

    {"main", IdentifierType::BuiltInFunctionName},
    {"print", IdentifierType::BuiltInFunctionName},
    {"println", IdentifierType::BuiltInFunctionName},
    {"sizeof", IdentifierType::BuiltInFunctionName}
};

struct IdentifierToken : Token<TokenType::Identifier> {
    IdentifierType identifier_type;

    IdentifierToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          identifier_type(mapIdentifierType(lexeme))
    {}

    static IdentifierType mapIdentifierType(const std::string& lexeme) {
        auto it = identifier_map.find(lexeme);
        if (it != identifier_map.end()) {
            return it->second;
        }

        return IdentifierType::UserDefined;
    }
};
