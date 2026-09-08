
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

struct CommentToken : Token<TokenType::Comment> {
    CommentType comment_type;
};
