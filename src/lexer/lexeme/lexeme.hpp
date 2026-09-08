
#pragma once

#include <string>
#include <cstddef>

namespace lexemizer {

    enum class LexemeType {
        Word,
        IntegerLiteral,
        FloatLiteral,
        CharacterLiteral,
        StringLiteral,
        Symbol,
        Whitespace,
        Comment
    };

    struct Lexeme {
        LexemeType type;
        std::string text;
        std::size_t line;
        std::size_t column;
    };

} // namespace lexemizer
