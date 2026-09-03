#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedEnumDefinition : ParsedPhrase {
    ParsedEnumDefinition(
        const std::string& name,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::EnumDefinition, source_phrase),
        name(name) {
    }

    std::string name;
    std::string base_type;
};