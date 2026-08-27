#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedReturnStatement : ParsedPhrase {
    ParsedReturnStatement(
        const std::string& expression,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ReturnStatement, source_phrase),
        expression(expression) {
    }

    std::string expression;
};