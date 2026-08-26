#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedAssignment : ParsedPhrase {
    ParsedAssignment(
        const std::string& left,
        const std::string& right,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::Assignment, source_phrase),
        left(left),
        right(right) {
    }

    std::string left;
    std::string right;
};