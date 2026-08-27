
#pragma once

#include <vector>
#include <memory>
#include <string>

#include "../parsed_phrase.cpp"

struct ParsedTemplateParameter {
    std::string name;
    std::vector<std::string> allowed_types;
    std::string category;
};

struct ParsedFunction : ParsedPhrase {
    ParsedFunction(
        const std::string& name,
        const std::string& return_type,
        const std::vector<ParsedTemplateParameter>& template_parameters = {},
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::Function, source_phrase),
        name(name),
        return_type(return_type),
        template_parameters(template_parameters) {
    }

    std::string name;
    std::string return_type;
    std::vector<ParsedTemplateParameter> template_parameters;
    std::vector<std::shared_ptr<ParsedPhrase>> parameters;
    bool is_unsafe = false;
};
