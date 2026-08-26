#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedParameterDefinition : ParsedPhrase {
    ParsedParameterDefinition(
        const std::string& type_name,
        const std::string& name,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ParameterDefinition, source_phrase),
        type_name(type_name),
        name(name) {
    }

    std::string type_name;
    std::string name;
};