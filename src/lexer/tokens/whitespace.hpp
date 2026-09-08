
#pragma once
 
#include "token.hpp"

enum class WhiteSpaceType {
    Space,
    Tab,
    NewLine,
    CarriageReturn
};

struct WhiteSpaceToken : Token<TokenType::Whitespace> {
    WhiteSpaceType white_space_type;
};
