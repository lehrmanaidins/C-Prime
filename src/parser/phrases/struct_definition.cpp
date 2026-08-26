#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedStructDefinition : ParsedPhrase {
    ParsedStructDefinition(
        const std::string& name,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::StructDefinition, source_phrase),
        name(name) {
    }

    std::string name;
};