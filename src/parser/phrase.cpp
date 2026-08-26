
#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "../lexer/token.cpp"

struct Phrase {
    Phrase(const std::vector<std::shared_ptr<Token>>& tokens,
           const std::shared_ptr<Phrase>& parent_phrase = nullptr
    ) : tokens(tokens), parent_phrase(parent_phrase) {

    }

    std::vector<std::shared_ptr<Token>> tokens;
    std::shared_ptr<Phrase> parent_phrase;
    std::vector<std::shared_ptr<Phrase>> nested_phrases;

    void addNested(const std::shared_ptr<Phrase>& nested) {
        nested_phrases.push_back(nested);
    }

    bool hasParent() const {
        return parent_phrase != nullptr;
    }
    
    bool hasNested() const {
        return !nested_phrases.empty();
    }
};

using Phrases = std::vector<std::shared_ptr<Phrase>>;
