#pragma once

#include <string>

#include "../parsed_phrase.cpp"

struct ParsedUnknownPhrase : ParsedPhrase {
    explicit ParsedUnknownPhrase(
        const std::string& text = "",
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::Unknown, source_phrase),
        text(text) {
    }

    std::string text;
};

struct ParsedBreakStatement : ParsedPhrase {
    explicit ParsedBreakStatement(
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::BreakStatement, source_phrase) {
    }
};

struct ParsedContinueStatement : ParsedPhrase {
    explicit ParsedContinueStatement(
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ContinueStatement, source_phrase) {
    }
};