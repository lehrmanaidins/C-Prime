
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class IdentifierType {
    Unknown,
    BuiltInPrimitiveType,
    BuiltInOtherType,
    BuiltInFunctionName,
    BuiltInVariableName,
    UserDefined,
};

static const std::unordered_map<IdentifierType, std::string_view> identifier_type_to_string = {
    {IdentifierType::Unknown, "Unknown"},
    {IdentifierType::BuiltInPrimitiveType, "BuiltInPrimitiveType"},
    {IdentifierType::BuiltInOtherType, "BuiltInOtherType"},
    {IdentifierType::BuiltInFunctionName, "BuiltInFunctionName"},
    {IdentifierType::BuiltInVariableName, "BuiltInVariableName"},
    {IdentifierType::UserDefined, "UserDefined"},
};

std::string_view toString(IdentifierType identifier_type) {
    return identifier_type_to_string.at(identifier_type);
}

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
    {"sizeof", IdentifierType::BuiltInFunctionName},

    {"UINT8_MAX", IdentifierType::BuiltInVariableName},
    {"UINT16_MAX", IdentifierType::BuiltInVariableName},
    {"UINT32_MAX", IdentifierType::BuiltInVariableName},
    {"UINT64_MAX", IdentifierType::BuiltInVariableName},
    {"INT8_MAX", IdentifierType::BuiltInVariableName},
    {"INT16_MAX", IdentifierType::BuiltInVariableName},
    {"INT32_MAX", IdentifierType::BuiltInVariableName},
    {"INT64_MAX", IdentifierType::BuiltInVariableName},
    {"UINT8_MIN", IdentifierType::BuiltInVariableName},
    {"UINT16_MIN", IdentifierType::BuiltInVariableName},
    {"UINT32_MIN", IdentifierType::BuiltInVariableName},
    {"UINT64_MIN", IdentifierType::BuiltInVariableName},
    {"INT8_MIN", IdentifierType::BuiltInVariableName},
    {"INT16_MIN", IdentifierType::BuiltInVariableName},
    {"INT32_MIN", IdentifierType::BuiltInVariableName},
    {"INT64_MIN", IdentifierType::BuiltInVariableName},
    {"FLOAT32_MAX", IdentifierType::BuiltInVariableName},
    {"FLOAT64_MAX", IdentifierType::BuiltInVariableName},
    {"FLOAT32_MIN", IdentifierType::BuiltInVariableName},
    {"FLOAT64_MIN", IdentifierType::BuiltInVariableName},
    {"CHAR8_MAX", IdentifierType::BuiltInVariableName},
    {"CHAR16_MAX", IdentifierType::BuiltInVariableName},
    {"CHAR32_MAX", IdentifierType::BuiltInVariableName},
    {"CHAR8_MIN", IdentifierType::BuiltInVariableName},
    {"CHAR16_MIN", IdentifierType::BuiltInVariableName},
    {"CHAR32_MIN", IdentifierType::BuiltInVariableName}
};

struct IdentifierToken : Token<TokenType::Identifier> {
    IdentifierType identifier_type;

    IdentifierToken(const Lexeme& lexeme)
        : Token<TokenType::Identifier>(lexeme),
          identifier_type(mapIdentifierType(lexeme))
    {}

    static IdentifierType mapIdentifierType(const Lexeme& lexeme) {
        auto it = identifier_map.find(lexeme.lexeme_text);
        if (it != identifier_map.end()) {
            return it->second;
        }

        return IdentifierType::UserDefined;
    }
};

std::string toString(const IdentifierToken& token) {
    std::string lexeme_str = "IdentifierToken = {";
    lexeme_str += std::string(toString(token.identifier_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
