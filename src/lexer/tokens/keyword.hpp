
#pragma once
 
#include "token.hpp"

enum class KeywordType {
    Type,
    Alias,
    Const,
    Mutable,
    Primitive,
    Struct,
    Interface,
    Enum,
    Union,
    Function,
    Template,
    Return,
    Requires,
    Ensures,
    If,
    Else,
    Switch,
    Case,
    For,
    ForEach,
    Do,
    While,
    Loop,
    Limit,
    Continue,
    Break,
    True,
    False,
    Import,
    Unsafe,
    Null,
    Void,
    Pure,
    Entry,
    Critical,
    Interrupt,
    Discard
};

struct KeywordToken : Token<TokenType::Keyword> {
    KeywordType keyword_type;
};
