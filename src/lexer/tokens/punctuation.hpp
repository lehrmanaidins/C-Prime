
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

#include "token.hpp"

enum class PunctuationType {
    Comma,
    Semicolon
};

struct PunctuationToken : Token<TokenType::Punctuation> {
    PunctuationType punctuation_type;
};
