
#pragma once

#include <string>
#include <cstddef>

namespace lexemizer {

    enum class LexemeType {
        WORD,
        INTEGER_LITERAL,
        FLOAT_LITERAL,
        CHARACTER_LITERAL,
        STRING_LITERAL,
        SYMBOL,
        WHITESPACE,
        COMMENT
    };

    struct Lexeme {
        LexemeType type;
        std::string text;
        std::size_t line;
        std::size_t column;
    };

} // namespace lexemizer
