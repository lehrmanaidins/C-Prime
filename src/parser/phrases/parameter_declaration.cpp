
#pragma once

#include <vector>
#include <memory>
#include <string>

#include "../parsed_phrase.cpp"

struct ParsedParameterDeclaration : ParsedPhrase {
    ParsedParameterDeclaration(
        const std::string& type_name,
        const std::string& name,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ParameterDeclaration, source_phrase),
        name(name),
        type_name(type_name) {
    }

    std::string name;
    std::string type_name;
};
