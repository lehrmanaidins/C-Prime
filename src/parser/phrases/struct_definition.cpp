#pragma once

#include <string>
#include <vector>

#include "../parsed_phrase.cpp"
#include "parsed_template_parameter.cpp"

struct ParsedStructDefinition : ParsedPhrase {
    ParsedStructDefinition(
        const std::string& name,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::StructDefinition, source_phrase),
        name(name) {
    }

    std::string name;
    std::vector<ParsedTemplateParameter> template_parameters;
};