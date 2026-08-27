#pragma once

#include <string>
#include <vector>

#include "../parsed_phrase.cpp"

struct ParsedUnionDefinition : ParsedPhrase {
    ParsedUnionDefinition(
        const std::string& name,
        const std::vector<std::string>& members,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::UnionDefinition, source_phrase),
        name(name),
        members(members) {
    }

    std::string name;
    std::vector<std::string> members;
};