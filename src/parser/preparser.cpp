
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <fstream>
#include <print>

#include "../lexer/token.cpp"
#include "phrase.cpp"

Phrases preparseTokens(const Tokens& tokens) {
    Phrases phrases{};
    std::vector<std::shared_ptr<Phrase>> scope_stack{};
    std::vector<std::shared_ptr<Token>> current_tokens{};

    auto flushCurrent = [&]() {
        if (current_tokens.empty()) {
            return;
        }

        std::shared_ptr<Phrase> parent = scope_stack.empty() ? nullptr : scope_stack.back();

        if (parent != nullptr && parent->tokens.empty() && parent->nested_phrases.empty()) {
            parent->tokens = current_tokens;
            current_tokens.clear();

            if (parent->hasParent()) {
                parent->parent_phrase->addNested(parent);
            } else {
                phrases.push_back(parent);
            }
            return;
        }

        auto phrase = std::make_shared<Phrase>(current_tokens, parent);

        if (parent != nullptr) {
            parent->addNested(phrase);
        } else {
            phrases.push_back(phrase);
        }

        current_tokens.clear();
    };

    auto openScope = [&]() {
        std::shared_ptr<Phrase> parent = scope_stack.empty()
            ? (phrases.empty() ? nullptr : phrases.back())
            : scope_stack.back();

        auto scope_phrase = std::make_shared<Phrase>(std::vector<std::shared_ptr<Token>>{}, parent);
        scope_stack.push_back(scope_phrase);
    };

    for (const std::shared_ptr<Token>& token : tokens) {
        if (!token) {
            continue;
        }

        const std::string value = token->value;

        if (value == "{") {
            flushCurrent();
            openScope();
            continue;
        }

        if (value == "}") {
            flushCurrent();
            if (!scope_stack.empty()) {
                scope_stack.pop_back();
            }
            continue;
        }

        if (value == ";") {
            flushCurrent();
            continue;
        }

        current_tokens.push_back(token);
    }

    flushCurrent();

    return phrases;
}
