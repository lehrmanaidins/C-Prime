
#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "../lexer/token.cpp"

struct Phrase {
    Phrase(const std::vector<std::shared_ptr<Token>>& tokens,
           const std::shared_ptr<Phrase>& parent_phrase = nullptr
    ) : tokens(tokens), parent_phrase(parent_phrase), start_line(0), start_column(0), end_line(0), end_column(0) {
        if (!this->tokens.empty() && this->tokens.front() && this->tokens.back()) {
            start_line = this->tokens.front()->line;
            start_column = this->tokens.front()->column;

            end_line = this->tokens.back()->line;
            end_column = this->tokens.back()->column;
            if (!this->tokens.back()->value.empty()) {
                end_column += this->tokens.back()->value.size() - 1;
            }
        }

    }

    std::vector<std::shared_ptr<Token>> tokens;
    std::shared_ptr<Phrase> parent_phrase;
    std::vector<std::shared_ptr<Phrase>> nested_phrases;
    size_t start_line;
    size_t start_column;
    size_t end_line;
    size_t end_column;

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
