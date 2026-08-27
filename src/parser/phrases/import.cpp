#pragma once

#include <string>

#include "../parsed_phrase.cpp"

enum class ParsedImportKind {
    CPrime,
    Cpp
};

struct ParsedImportStatement : ParsedPhrase {
    ParsedImportStatement(
        const std::string& path,
        ParsedImportKind import_kind,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::ImportStatement, source_phrase),
        path(path),
        import_kind(import_kind) {
    }

    std::string path;
    ParsedImportKind import_kind;
};