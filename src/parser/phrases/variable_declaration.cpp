#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedVariableDeclaration : ParsedPhrase {
    ParsedVariableDeclaration(
        const std::string& type_name,
        const std::string& name,
        const std::string& initializer,
        bool is_mutable,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::VariableDeclaration, source_phrase),
        type_name(type_name),
        name(name),
        initializer(initializer),
        is_mutable(is_mutable) {
    }

    std::string type_name;
    std::string name;
    std::string initializer;
    bool is_mutable;
};