
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../../error.hpp"

enum class IdentifierTokenType {
    Unknown,
    BuiltInPrimitiveType,
    BuiltInOtherType,
    BuiltInFunctionName,
    BuiltInVariableName,
    UserDefined,
};

static const std::unordered_map<IdentifierTokenType, std::string> identifier_token_type_to_string = {
    {IdentifierTokenType::Unknown, "Unknown"},
    {IdentifierTokenType::BuiltInPrimitiveType, "BuiltInPrimitiveType"},
    {IdentifierTokenType::BuiltInOtherType, "BuiltInOtherType"},
    {IdentifierTokenType::BuiltInFunctionName, "BuiltInFunctionName"},
    {IdentifierTokenType::BuiltInVariableName, "BuiltInVariableName"},
    {IdentifierTokenType::UserDefined, "UserDefined"},
};

static const std::unordered_map<std::string, IdentifierTokenType> identifier_map = {
    {"uint8", IdentifierTokenType::BuiltInPrimitiveType},
    {"uint32", IdentifierTokenType::BuiltInPrimitiveType},
    {"uint64", IdentifierTokenType::BuiltInPrimitiveType},
    {"uint64", IdentifierTokenType::BuiltInPrimitiveType},
    {"int8", IdentifierTokenType::BuiltInPrimitiveType},
    {"int16", IdentifierTokenType::BuiltInPrimitiveType},
    {"int32", IdentifierTokenType::BuiltInPrimitiveType},
    {"int64", IdentifierTokenType::BuiltInPrimitiveType},
    {"float32", IdentifierTokenType::BuiltInPrimitiveType},
    {"float64", IdentifierTokenType::BuiltInPrimitiveType},
    {"char8", IdentifierTokenType::BuiltInPrimitiveType},
    {"char16", IdentifierTokenType::BuiltInPrimitiveType},
    {"char32", IdentifierTokenType::BuiltInPrimitiveType},
    {"bool", IdentifierTokenType::BuiltInPrimitiveType},

    {"void", IdentifierTokenType::BuiltInOtherType},
    {"pointer", IdentifierTokenType::BuiltInOtherType},
    {"reference", IdentifierTokenType::BuiltInOtherType},

    {"main", IdentifierTokenType::BuiltInFunctionName},
    {"print", IdentifierTokenType::BuiltInFunctionName},
    {"println", IdentifierTokenType::BuiltInFunctionName},
    {"sizeof", IdentifierTokenType::BuiltInFunctionName},

    {"UINT8_MAX", IdentifierTokenType::BuiltInVariableName},
    {"UINT16_MAX", IdentifierTokenType::BuiltInVariableName},
    {"UINT32_MAX", IdentifierTokenType::BuiltInVariableName},
    {"UINT64_MAX", IdentifierTokenType::BuiltInVariableName},
    {"INT8_MAX", IdentifierTokenType::BuiltInVariableName},
    {"INT16_MAX", IdentifierTokenType::BuiltInVariableName},
    {"INT32_MAX", IdentifierTokenType::BuiltInVariableName},
    {"INT64_MAX", IdentifierTokenType::BuiltInVariableName},
    {"UINT8_MIN", IdentifierTokenType::BuiltInVariableName},
    {"UINT16_MIN", IdentifierTokenType::BuiltInVariableName},
    {"UINT32_MIN", IdentifierTokenType::BuiltInVariableName},
    {"UINT64_MIN", IdentifierTokenType::BuiltInVariableName},
    {"INT8_MIN", IdentifierTokenType::BuiltInVariableName},
    {"INT16_MIN", IdentifierTokenType::BuiltInVariableName},
    {"INT32_MIN", IdentifierTokenType::BuiltInVariableName},
    {"INT64_MIN", IdentifierTokenType::BuiltInVariableName},
    {"FLOAT32_MAX", IdentifierTokenType::BuiltInVariableName},
    {"FLOAT64_MAX", IdentifierTokenType::BuiltInVariableName},
    {"FLOAT32_MIN", IdentifierTokenType::BuiltInVariableName},
    {"FLOAT64_MIN", IdentifierTokenType::BuiltInVariableName},
    {"CHAR8_MAX", IdentifierTokenType::BuiltInVariableName},
    {"CHAR16_MAX", IdentifierTokenType::BuiltInVariableName},
    {"CHAR32_MAX", IdentifierTokenType::BuiltInVariableName},
    {"CHAR8_MIN", IdentifierTokenType::BuiltInVariableName},
    {"CHAR16_MIN", IdentifierTokenType::BuiltInVariableName},
    {"CHAR32_MIN", IdentifierTokenType::BuiltInVariableName}
};

struct IdentifierToken : Token {
    IdentifierTokenType identifier_type;

    IdentifierToken(const Lexeme& lexeme)
        : Token(lexeme),
          identifier_type(mapIdentifierTokenType(lexeme))
    {}

    static IdentifierTokenType mapIdentifierTokenType(const Lexeme& lexeme) {
        auto it = identifier_map.find(lexeme.text);
        if (it != identifier_map.end()) {
            return it->second;
        }

        return IdentifierTokenType::UserDefined;
    }

    std::string toString() const override {
        std::string lexeme_str = "IdentifierToken = {";
        lexeme_str += identifier_token_type_to_string.at(identifier_type) + ", ";
        lexeme_str += "\"" + lexeme.text + "\", ";
        lexeme_str += "(" + std::to_string(lexeme.location.line) + ", ";
        lexeme_str += std::to_string(lexeme.location.column) + ")}";
        return lexeme_str;
    }
};
