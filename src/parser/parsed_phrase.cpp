#pragma once

#include <memory>
#include <string>
#include <vector>

#include "phrase.cpp"

enum class ParsedPhraseKind {
    Unknown,
    Function,
    ParameterDeclaration,
    VariableDeclaration,
    TypeDefinition,
    StructDefinition,
    EnumDefinition,
    Assignment,
    ParameterDefinition,
    EnumValueDefinition,
    CallStatement,
    ReturnStatement,
    BreakStatement,
    ContinueStatement,
    IfStatement,
    WhileStatement,
    ForStatement,
    ElseStatement
};

struct ParsedPhrase : std::enable_shared_from_this<ParsedPhrase> {
    explicit ParsedPhrase(
        ParsedPhraseKind kind = ParsedPhraseKind::Unknown,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : kind(kind), source_phrase(source_phrase) {
    }

    virtual ~ParsedPhrase() = default;

    ParsedPhraseKind kind;
    std::shared_ptr<Phrase> source_phrase;
    std::weak_ptr<ParsedPhrase> parent_phrase;
    std::vector<std::shared_ptr<ParsedPhrase>> nested_phrases;

    void addNested(const std::shared_ptr<ParsedPhrase>& nested) {
        if (!nested) {
            return;
        }

        nested->parent_phrase = shared_from_this();
        nested_phrases.push_back(nested);
    }

    virtual std::string label() const {
        switch (kind) {
            case ParsedPhraseKind::Function:
                return "Function";
            case ParsedPhraseKind::ParameterDeclaration:
                return "ParameterDeclaration";
            case ParsedPhraseKind::VariableDeclaration:
                return "VariableDeclaration";
            case ParsedPhraseKind::TypeDefinition:
                return "TypeDefinition";
            case ParsedPhraseKind::StructDefinition:
                return "StructDefinition";
            case ParsedPhraseKind::EnumDefinition:
                return "EnumDefinition";
            case ParsedPhraseKind::Assignment:
                return "Assignment";
            case ParsedPhraseKind::ParameterDefinition:
                return "ParameterDefinition";
            case ParsedPhraseKind::EnumValueDefinition:
                return "EnumValueDefinition";
            case ParsedPhraseKind::CallStatement:
                return "CallStatement";
            case ParsedPhraseKind::ReturnStatement:
                return "ReturnStatement";
            case ParsedPhraseKind::BreakStatement:
                return "BreakStatement";
            case ParsedPhraseKind::ContinueStatement:
                return "ContinueStatement";
            case ParsedPhraseKind::IfStatement:
                return "IfStatement";
            case ParsedPhraseKind::WhileStatement:
                return "WhileStatement";
            case ParsedPhraseKind::ForStatement:
                return "ForStatement";
            case ParsedPhraseKind::ElseStatement:
                return "ElseStatement";
            case ParsedPhraseKind::Unknown:
            default:
                return "Unknown";
        }
    }

};

struct ParsedCallStatement : ParsedPhrase {
    ParsedCallStatement(
        const std::string& name,
        const std::vector<std::string>& arguments,
        const std::shared_ptr<Phrase>& source_phrase = nullptr
    ) : ParsedPhrase(ParsedPhraseKind::CallStatement, source_phrase),
        name(name),
        arguments(arguments) {
    }

    std::string name;
    std::vector<std::string> arguments;
};

using ParsedPhrases = std::vector<std::shared_ptr<ParsedPhrase>>;