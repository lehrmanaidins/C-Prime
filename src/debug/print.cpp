#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "../lexer/token.cpp"
#include "../parser/parsed_phrase.cpp"
#include "../parser/phrase.cpp"

static std::string tokenCategoryName(TokenCatagory category) {
    switch (category) {
        case TokenCatagory::Default:
            return "Default";
        case TokenCatagory::Separator:
            return "Separator";
        case TokenCatagory::Keyword:
            return "Keyword";
        case TokenCatagory::Primitive:
            return "Primitive";
        case TokenCatagory::Type:
            return "Type";
        case TokenCatagory::Identifier:
            return "Identifier";
        case TokenCatagory::Literal:
            return "Literal";
        case TokenCatagory::Operator:
            return "Operator";
        case TokenCatagory::Unknown:
        default:
            return "Unknown";
    }
}

static std::string joinTokenText(const Tokens& tokens) {
    std::string text;

    for (const auto& token : tokens) {
        if (!token) {
            continue;
        }

        if (!text.empty()) {
            text += " ";
        }

        text += token->value;
    }

    return text;
}

static void printTokens(const Tokens& tokens) {
    std::cout << "Found " << tokens.size() << " tokens:\n";

    for (const auto& token : tokens) {
        std::cout << "\ttoken type: " << tokenCategoryName(token->type)
                  << "\tloc:\t" << token->line << ":" << token->column
                  << "\ttoken:\t" << token->value << "\n";
    }
}

static void printPhraseTree(const std::shared_ptr<Phrase>& phrase, int depth = 0) {
    std::cout << std::string(depth * 2, ' ') << "Phrase ["
              << phrase->start_line << ":" << phrase->start_column
              << "-" << phrase->end_line << ":" << phrase->end_column
              << "]: " << joinTokenText(phrase->tokens) << "\n";
    for (const auto& token : phrase->tokens) {
        std::cout << std::string(depth * 2 + 2, ' ') << "  "
                  << tokenCategoryName(token->type) << " ["
                  << token->line << ":" << token->column << "] : "
                  << token->value << "\n";
    }

    for (const auto& nested : phrase->nested_phrases) {
        printPhraseTree(nested, depth + 1);
    }
}

static void printPhrases(const Phrases& phrases) {
    std::cout << "Found " << phrases.size() << " phrases:\n";
    for (const auto& phrase : phrases) {
        printPhraseTree(phrase);
    }
}

static void printParsedPhraseTree(const std::shared_ptr<ParsedPhrase>& phrase, int depth = 0) {
    if (!phrase) {
        return;
    }

    const std::string phrase_text = phrase->source_phrase
        ? joinTokenText(phrase->source_phrase->tokens)
        : "";
    std::cout << std::string(depth * 2, ' ') << phrase->label() << ": " << phrase_text << "\n";
    for (const auto& nested : phrase->nested_phrases) {
        printParsedPhraseTree(nested, depth + 1);
    }
}

static void printParsedPhrases(const ParsedPhrases& parsed_phrases) {
    std::cout << "Found " << parsed_phrases.size() << " parsed phrases:\n";
    for (const auto& parsed_phrase : parsed_phrases) {
        printParsedPhraseTree(parsed_phrase);
    }
}

static void printSemanticSummary(const SemanticProgram& semantic_program) {
    std::cout << "Semantic analysis completed successfully.\n";
    std::cout << "Built semantic IR: "
              << semantic_program.functions.size() << " functions, "
              << semantic_program.type_definitions.size() << " type definitions, "
              << semantic_program.struct_definitions.size() << " struct definitions, "
              << semantic_program.enum_definitions.size() << " enum definitions\n";
}