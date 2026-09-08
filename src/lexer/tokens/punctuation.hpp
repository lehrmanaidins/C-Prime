
#pragma once
 
#include "token.hpp"

enum class PunctuationType {
    Comma,
    Semicolon
};

struct PunctuationToken : Token<TokenType::Punctuation> {
    PunctuationType punctuation_type;
};
