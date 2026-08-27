#pragma once

#include "../parsed_phrase.cpp"

struct ParsedUnsafeBlock : ParsedPhrase {
    explicit ParsedUnsafeBlock(const std::shared_ptr<Phrase>& source_phrase = nullptr)
        : ParsedPhrase(ParsedPhraseKind::UnsafeBlock, source_phrase) {
    }
};
