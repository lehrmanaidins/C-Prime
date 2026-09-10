
#pragma once

#include <variant>
#include <string>

#include "keyword_token.hpp"
#include "identifier_token.hpp"
#include "integer_literal_token.hpp"
#include "float_literal_token.hpp"
#include "character_literal_token.hpp"
#include "string_literal_token.hpp"
#include "operator_token.hpp"
#include "punctuation_token.hpp"
#include "whitespace_token.hpp"
#include "comment_token.hpp"

using TokenVariant = std::variant<
    KeywordToken,
    IdentifierToken,
    IntegerLiteralToken,
    FloatLiteralToken,
    CharacterLiteralToken,
    StringLiteralToken,
    OperatorToken,
    PunctuationToken,
    WhiteSpaceToken,
    CommentToken
>;

std::string toString(const TokenVariant& token) {
    return std::visit([](const auto& t) { return t.toString(); }, token);
}
