#pragma once

#include <string>
#include <vector>

#include "../parsed_phrase.cpp"
#include "parsed_template_parameter.cpp"

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
    std::vector<ParsedTemplateParameter> template_parameters;
};

struct ParsedRequiresClause : ParsedPhrase {
    ParsedRequiresClause(
        const std::string& expression,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::RequiresClause, source_phrase),
        expression(expression) {
    }

    std::string expression;
};

struct ParsedEnsuresClause : ParsedPhrase {
    ParsedEnsuresClause(
        const std::string& expression,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::EnsuresClause, source_phrase),
        expression(expression) {
    }

    std::string expression;
};