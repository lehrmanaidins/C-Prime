
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"
#include "../lexer_error.hpp"

enum class PunctuationType {
    Comma,
    Semicolon
};

struct PunctuationToken : Token<TokenType::Punctuation> {
    PunctuationType punctuation_type;
};
