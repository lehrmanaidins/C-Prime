
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
        || tokens[start]->value == "foreach"
        || tokens[start]->value == "do"
        || tokens[start]->value == "loop"
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

static bool isPendingFunctionContractHeader(const std::vector<std::shared_ptr<Token>>& tokens) {
    bool has_function = false;
    bool has_contract_clause = false;
    for (const auto& token : tokens) {
        if (!token) {
            continue;
        }
        has_function = has_function || token->value == "function";
        has_contract_clause = has_contract_clause || token->value == "requires" || token->value == "ensures";
    }
    return has_function && has_contract_clause;
}

Phrases preparseTokens(const Tokens& tokens) {
    Phrases phrases{};
    std::vector<std::shared_ptr<Phrase>> scope_stack{};
    std::vector<std::shared_ptr<Token>> current_tokens{};
    int expression_brace_depth = 0;
    int paren_depth = 0;

    // Comment and newline tokens are stripped from phrases here and preserved
    // as trivia so the transpiler can reproduce blank lines and comments.
    std::vector<PhraseTrivia> pending_trivia{};
    std::string pending_trailing_comment{};
    std::shared_ptr<Phrase> last_flushed_phrase{};
    bool line_has_content = false;

    auto flushCurrent = [&]() {
        if (current_tokens.empty()) {
            return;
        }

        std::shared_ptr<Phrase> parent = scope_stack.empty() ? nullptr : scope_stack.back();
        auto phrase = std::make_shared<Phrase>(current_tokens, parent);
        phrase->leading_trivia = std::move(pending_trivia);
        pending_trivia.clear();
        phrase->trailing_comment = pending_trailing_comment;
        pending_trailing_comment.clear();

        if (parent != nullptr) {
            parent->addNested(phrase);
        } else {
            phrases.push_back(phrase);
        }

        last_flushed_phrase = phrase;
        current_tokens.clear();
    };

    for (const std::shared_ptr<Token>& token : tokens) {
        if (!token) {
            continue;
        }

        if (token->type == TokenCatagory::Newline) {
            if (!line_has_content) {
                pending_trivia.push_back({PhraseTriviaKind::BlankLine, ""});
            }
            line_has_content = false;
            continue;
        }

        if (token->type == TokenCatagory::Comment) {
            line_has_content = true;
            if (current_tokens.empty()
                && last_flushed_phrase != nullptr
                && last_flushed_phrase->end_line == token->line
                && last_flushed_phrase->trailing_comment.empty()) {
                last_flushed_phrase->trailing_comment = token->value;
            } else if (!current_tokens.empty()
                && current_tokens.back()
                && current_tokens.back()->line == token->line) {
                pending_trailing_comment = token->value;
            } else {
                pending_trivia.push_back({PhraseTriviaKind::Comment, token->value});
            }
            continue;
        }

        line_has_content = true;

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
                if (!pending_trivia.empty()) {
                    scope_stack.back()->trailing_trivia = std::move(pending_trivia);
                    pending_trivia.clear();
                }
                scope_stack.pop_back();
            }
            continue;
        }

        if (value == ";") {
            if (expression_brace_depth > 0 || paren_depth > 0) {
                current_tokens.push_back(token);
                continue;
            }

            if (isPendingFunctionContractHeader(current_tokens)) {
                current_tokens.push_back(token);
                continue;
            }

            flushCurrent();
            continue;
        }

        current_tokens.push_back(token);
    }

    flushCurrent();

    if (!pending_trivia.empty() && last_flushed_phrase != nullptr) {
        for (auto& trivia : pending_trivia) {
            last_flushed_phrase->trailing_trivia.push_back(std::move(trivia));
        }
        pending_trivia.clear();
    }

    return phrases;
}
