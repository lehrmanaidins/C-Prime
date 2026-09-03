#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedIfStatement : ParsedPhrase {
    ParsedIfStatement(
        const std::string& condition,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::IfStatement, source_phrase),
        condition(condition) {
    }

    std::string condition;
};

struct ParsedWhileStatement : ParsedPhrase {
    ParsedWhileStatement(
        const std::string& condition,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::WhileStatement, source_phrase),
        condition(condition) {
    }

    std::string condition;
    std::string limit;  // empty when no `limit (...)` clause is present
};

struct ParsedForStatement : ParsedPhrase {
    ParsedForStatement(
        const std::string& initializer,
        const std::string& condition,
        const std::string& update,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ForStatement, source_phrase),
        initializer(initializer),
        condition(condition),
        update(update) {
    }

    std::string initializer;
    std::string condition;
    std::string update;
    std::string limit;
};

// `foreach (ElementType element in collection) [limit (N)] { ... }`
struct ParsedForeachStatement : ParsedPhrase {
    ParsedForeachStatement(
        const std::string& element_type,
        const std::string& element_name,
        const std::string& collection,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ForeachStatement, source_phrase),
        element_type(element_type),
        element_name(element_name),
        collection(collection) {
    }

    std::string element_type;
    std::string element_name;
    std::string collection;
    std::string limit;
};

// `do { ... } while (condition) [limit (N)];` — the condition is filled in by a
// post-parse pass that folds the trailing `while (...)` sibling into this node.
struct ParsedDoWhileStatement : ParsedPhrase {
    explicit ParsedDoWhileStatement(
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::DoWhileStatement, source_phrase) {
    }

    std::string condition;
    std::string limit;
};

// `loop [limit (N)] { ... }` — an endless loop, ended only by `break` or `limit`.
struct ParsedLoopStatement : ParsedPhrase {
    explicit ParsedLoopStatement(
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::LoopStatement, source_phrase) {
    }

    std::string limit;
};

struct ParsedElseStatement : ParsedPhrase {
    ParsedElseStatement(
        const std::string& condition = "",
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ElseStatement, source_phrase),
        condition(condition) {
    }

    std::string condition;
};