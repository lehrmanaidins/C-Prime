
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class CommentTokenType {
    Unknown,
    SingleLine,
    MultiLine
};

static const std::unordered_map<CommentTokenType, std::string> comment_token_type_to_string = {
    {CommentTokenType::Unknown, "Unknown"},
    {CommentTokenType::SingleLine, "SingleLine"},
    {CommentTokenType::MultiLine, "MultiLine"},
};

static const std::unordered_map<std::string, CommentTokenType> comment_map = {
    {"//", CommentTokenType::SingleLine},
    {"/*", CommentTokenType::MultiLine}
};

struct CommentToken : Token {
    CommentTokenType comment_type;

    CommentToken(const Lexeme& lexeme)
        : Token(lexeme),
          comment_type(mapCommentTokenType(lexeme))
    {}

    static CommentTokenType mapCommentTokenType(const Lexeme& lexeme) {
        auto it = comment_map.find(lexeme.lexeme_text.substr(0, 2));
        if (it != comment_map.end()) {
            return it->second;
        }
        printLexemeError("unknown comment" + lexeme.lexeme_text, lexeme);
        return CommentTokenType::Unknown;
    }

    std::string toString() const override {
        std::string lexeme_str = "CommentToken = {";
        lexeme_str += comment_token_type_to_string.at(comment_type) + ", ";
        lexeme_str += "\"" + lexeme_text + "\", ";
        lexeme_str += "(" + std::to_string(line_number) + ", ";
        lexeme_str += std::to_string(column_number) + ")}";
        return lexeme_str;
    }
};
