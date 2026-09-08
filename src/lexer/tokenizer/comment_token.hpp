
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class CommentType {
    Unknown,
    SingleLine,
    MultiLine
};

static const std::unordered_map<CommentType, std::string_view> comment_type_to_string = {
    {CommentType::Unknown, "Unknown"},
    {CommentType::SingleLine, "SingleLine"},
    {CommentType::MultiLine, "MultiLine"},
};

std::string_view toString(CommentType comment_type) {
    return comment_type_to_string.at(comment_type);
}

static const std::unordered_map<std::string, CommentType> comment_map = {
    {"//", CommentType::SingleLine},
    {"/*", CommentType::MultiLine}
};

struct CommentToken : Token<TokenType::Comment> {
    CommentType comment_type;

    CommentToken(const Lexeme& lexeme)
        : Token<TokenType::Comment>(lexeme),
          comment_type(mapCommentType(lexeme))
    {}

    static CommentType mapCommentType(const Lexeme& lexeme) {
        auto it = comment_map.find(lexeme.lexeme_text.substr(0, 2));
        if (it != comment_map.end()) {
            return it->second;
        }
        throwLexerError("Unknown Comment: " + lexeme.lexeme_text, lexeme.line_text, lexeme.line_number, lexeme.column_number);
        return CommentType::Unknown;
    }
};

std::string toString(const CommentToken& token) {
    std::string lexeme_str = "CommentToken = {";
    lexeme_str += std::string(toString(token.comment_type)) + ", ";
    lexeme_str += "\"" + token.lexeme_text + "\", ";
    lexeme_str += "(" + std::to_string(token.line_number) + ", ";
    lexeme_str += std::to_string(token.column_number) + ")}";
    return lexeme_str;
}