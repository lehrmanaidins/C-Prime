#pragma once

#include <string>
#include <vector>

#include "../parsed_phrase.cpp"

struct ParsedEnumValueDefinition : ParsedPhrase {
    ParsedEnumValueDefinition(
        const std::vector<std::string>& values,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::EnumValueDefinition, source_phrase),
        values(values) {
    }

    std::vector<std::string> values;
};