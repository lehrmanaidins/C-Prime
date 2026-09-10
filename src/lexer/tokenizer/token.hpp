
#pragma once

#include <string>
#include <unordered_map>

#include "../lexemizer/lexeme.hpp"
#include "../../source/source_location.hpp"

struct Token {
    Lexeme lexeme; 

    Token(const Lexeme& lexeme)
        : lexeme(lexeme)
    {}

    virtual ~Token() = default;

    virtual std::string toString() const = 0;
};
