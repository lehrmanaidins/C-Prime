
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class CommentType {
    SingleLine,
    MultiLine
};

static const std::unordered_map<std::string, CommentType> comment_map = {
    {"//", CommentType::SingleLine},
    {"/*", CommentType::MultiLine}
};

struct CommentToken : Token<TokenType::Comment> {
    CommentType comment_type;

    CommentToken(std::string lexeme, std::size_t line, std::size_t column)
        : Token(lexeme, line, column),
          comment_type(mapCommentType(lexeme, line, column))
    {}

    static CommentType mapCommentType(const std::string& lexeme, std::size_t line, std::size_t column) {
        auto it = comment_map.find(lexeme.substr(0, 2));
        if (it != comment_map.end()) {
            return it->second;
        }
        throwLexerError("Unknown Comment: " + lexeme, lexeme, line, column);
    }
};
