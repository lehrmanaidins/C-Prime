
#pragma once

#include <string>
#include <vector>

#include "../io.hpp"

#include "lexemizer/lexeme.hpp"
#include "lexemizer/lexemizer.hpp"

#include "tokenizer/token.hpp"
#include "tokenizer/tokenizer.hpp"

std::vector<TokenVariant> lexFile(const std::string& filename) {
    std::vector<std::string> lines = io::readFileLines(filename); 

    std::vector<Lexeme> lexemes = lexemize(lines);

    std::vector<TokenVariant> tokens = tokenize(lexemes);

    return tokens;
}
