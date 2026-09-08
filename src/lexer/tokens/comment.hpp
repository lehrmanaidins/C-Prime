
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class CommentType {
    SingleLine,
    MultiLine
};

struct CommentToken : Token<TokenType::Comment> {
    CommentType comment_type;
};
