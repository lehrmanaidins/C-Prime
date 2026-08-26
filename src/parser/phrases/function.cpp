
#pragma once

#include <vector>
#include <memory>
#include <string>

#include "../parsed_phrase.cpp"

struct ParsedFunction : ParsedPhrase {
    ParsedFunction(
        const std::string& name,
        const std::string& return_type,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::Function, source_phrase),
        name(name),
        return_type(return_type) {
    }

    std::string name;
    std::string return_type;
    std::vector<std::shared_ptr<ParsedPhrase>> parameters;
};
