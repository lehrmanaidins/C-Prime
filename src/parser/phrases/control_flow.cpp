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