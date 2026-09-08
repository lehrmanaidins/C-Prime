
#pragma once

#include <string>
#include <unordered_map>

#include "../lexemizer/lexeme.hpp"

enum class TokenType {
    Keyword,
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    CharacterLiteral,
    StringLiteral,
    Operator,
    Punctuation,
    Comment,
    WhiteSpace,
};

static const std::unordered_map<TokenType, std::string> token_type_to_string = {
    {TokenType::Keyword, "Keyword"},
    {TokenType::Identifier, "Identifier"},
    {TokenType::IntegerLiteral, "IntegerLiteral"},
    {TokenType::FloatLiteral, "FloatLiteral"},
    {TokenType::CharacterLiteral, "CharacterLiteral"},
    {TokenType::StringLiteral, "StringLiteral"},
    {TokenType::Operator, "Operator"},
    {TokenType::Punctuation, "Punctuation"},
    {TokenType::Comment, "Comment"},
    {TokenType::WhiteSpace, "Whitespace"},
};

std::string toString(TokenType token_type) {
    return token_type_to_string.at(token_type);
}

template <TokenType T>
struct Token {
    static constexpr TokenType token_type = T;
    std::string lexeme_text;
    std::string line_text;
    std::size_t line_number;
    std::size_t column_number;

    Token(const Lexeme& lexeme)
        : lexeme_text(lexeme.lexeme_text),
          line_text(lexeme.line_text),
          line_number(lexeme.line_number),
          column_number(lexeme.column_number)
    {}
};

template <TokenType T>
std::string toString(const Token<T>& token) {
    std::string lexeme_str = "Token<" + toString(T) + "> = {";
    lexeme_str += toString(T) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}
