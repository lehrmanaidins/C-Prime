
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
 
#include "token.hpp"
#include "../lexer_error.hpp"

enum class WhiteSpaceType {
    Space,
    Tab,
    NewLine,
    CarriageReturn
};

struct WhiteSpaceToken : Token<TokenType::Whitespace> {
    WhiteSpaceType white_space_type;
};
