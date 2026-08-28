
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <fstream>
#include <print>

#include "../lexer/token.cpp"
#include "phrase.cpp"

static bool phraseIntroducesScope(const std::vector<std::shared_ptr<Token>>& tokens) {
    if (tokens.empty()) {
        return false;
    }

    size_t start = 0;
    while (start + 1 < tokens.size() && tokens[start] && tokens[start]->value == "@" && tokens[start + 1]) {
        start += 2;
    }

    if (start >= tokens.size()) {
        return false;
    }

    if (tokens[start] && (
        tokens[start]->value == "function"
        || tokens[start]->value == "template"
        || tokens[start]->value == "struct"
        || tokens[start]->value == "enum"
        || tokens[start]->value == "type"
        || tokens[start]->value == "if"
        || tokens[start]->value == "while"
        || tokens[start]->value == "for"
        || tokens[start]->value == "else"
    )) {
        return true;
    }

    if (tokens[start] && tokens[start]->value == "unsafe") {
        if (tokens.size() == start + 1) {
            return true;
        }
        return tokens[start + 1] && tokens[start + 1]->value == "function";
    }

    return false;
}

Phrases preparseTokens(const Tokens& tokens) {
    Phrases phrases{};
    std::vector<std::shared_ptr<Phrase>> scope_stack{};
    std::vector<std::shared_ptr<Token>> current_tokens{};
    int expression_brace_depth = 0;
    int paren_depth = 0;

    auto flushCurrent = [&]() {
        if (current_tokens.empty()) {
            return;
        }

        std::shared_ptr<Phrase> parent = scope_stack.empty() ? nullptr : scope_stack.back();
        auto phrase = std::make_shared<Phrase>(current_tokens, parent);

        if (parent != nullptr) {
            parent->addNested(phrase);
        } else {
            phrases.push_back(phrase);
        }

        current_tokens.clear();
    };

    for (const std::shared_ptr<Token>& token : tokens) {
        if (!token) {
            continue;
        }

        const std::string value = token->value;

        if (value == "(") {
            ++paren_depth;
            current_tokens.push_back(token);
            continue;
        }

        if (value == ")") {
            if (paren_depth > 0) {
                --paren_depth;
            }
            current_tokens.push_back(token);
            continue;
        }

        if (value == "{") {
            if (!current_tokens.empty() && phraseIntroducesScope(current_tokens)) {
                flushCurrent();
                if (!scope_stack.empty()) {
                    scope_stack.push_back(scope_stack.back()->nested_phrases.back());
                } else if (!phrases.empty()) {
                    scope_stack.push_back(phrases.back());
                }
                continue;
            }

            if (!scope_stack.empty() || !current_tokens.empty()) {
                current_tokens.push_back(token);
                ++expression_brace_depth;
                continue;
            }

            continue;
        }

        if (value == "}") {
            if (expression_brace_depth > 0) {
                current_tokens.push_back(token);
                --expression_brace_depth;
                continue;
            }

            flushCurrent();
            if (!scope_stack.empty()) {
                scope_stack.pop_back();
            }
            continue;
        }

        if (value == ";") {
            if (expression_brace_depth > 0 || paren_depth > 0) {
                current_tokens.push_back(token);
                continue;
            }

            flushCurrent();
            continue;
        }

        current_tokens.push_back(token);
    }

    flushCurrent();

    return phrases;
}
