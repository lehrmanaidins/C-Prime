#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedTypeDefinition : ParsedPhrase {
    ParsedTypeDefinition(
        const std::string& name,
        const std::string& base_type,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::TypeDefinition, source_phrase),
        name(name),
        base_type(base_type) {
    }

    std::string name;
    std::string base_type;
};