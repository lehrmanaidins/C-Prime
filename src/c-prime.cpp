
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <exception>

#include "lexer/lexer.cpp"
#include "parser/preparser.cpp"
#include "parser/parser.cpp"
#include "semantic/semantic.cpp"
#include "transpiler/cpp_emitter.cpp"

std::string tokenCategoryName(TokenCatagory category) {
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

std::string joinPhraseText(const Tokens& tokens) {
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

void printPhraseTree(const std::shared_ptr<Phrase>& phrase, int depth = 0) {
    std::cout << std::string(depth * 2, ' ') << "Phrase ["
              << phrase->start_line << ":" << phrase->start_column
              << "-" << phrase->end_line << ":" << phrase->end_column
              << "]: " << joinPhraseText(phrase->tokens) << "\n";
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

void printParsedPhraseTree(const std::shared_ptr<ParsedPhrase>& phrase, int depth = 0) {
    if (!phrase) {
        return;
    }

    const std::string phrase_text = phrase->source_phrase
        ? joinPhraseText(phrase->source_phrase->tokens)
        : "";
    std::cout << std::string(depth * 2, ' ') << phrase->label() << ": " << phrase_text << "\n";
    for (const auto& nested : phrase->nested_phrases) {
        printParsedPhraseTree(nested, depth + 1);
    }
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    const bool exists = file.is_open();
    file.close();
    return exists;
}

int main(int argc, char* argv[]) {
    bool debug = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--debug") {
            debug = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--debug] <filename>\n";
            return 0;
        } else if (filename.empty()) {
            filename = arg;
        } else {
            std::cout << "Usage: " << argv[0] << " [--debug] <filename>\n";
            return 1;
        }
    }

    if (filename.empty()) {
        std::cout << "Usage: " << argv[0] << " [--debug] <filename>\n";
        return 1;
    }

    std::cout << "Lexing file: " << filename << "\n";

    if (!fileExists(filename)) {
        std::cout << "Error: File " << filename << " does not exist.\n";
        return 1;
    }

    Tokens tokens = lexFile(filename);

    if (debug) {
        std::cout << "Found " << tokens.size() << " tokens:\n";

        for (const auto& token : tokens) {
            std::cout << "\ttoken type: " << tokenCategoryName(token->type)
                      << "\tloc:\t" << token->line << ":" << token->column
                      << "\ttoken:\t" << token->value << "\n";
        }
    }

    Phrases phrases = preparseTokens(tokens);

    if (debug) {
        std::cout << "Found " << phrases.size() << " phrases:\n";
        for (const auto& phrase : phrases) {
            printPhraseTree(phrase);
        }
    }

    try {
        ParsedPhrases parsed_phrases = parsePhrases(phrases);
        if (debug) {
            std::cout << "Found " << parsed_phrases.size() << " parsed phrases:\n";
            for (const auto& parsed_phrase : parsed_phrases) {
                printParsedPhraseTree(parsed_phrase);
            }
        }

        SemanticProgram semantic_program = buildSemanticProgram(parsed_phrases);
        std::cout << "Semantic analysis completed successfully.\n";
        std::cout << "Built semantic IR: "
                  << semantic_program.functions.size() << " functions, "
                  << semantic_program.type_definitions.size() << " type definitions, "
                  << semantic_program.struct_definitions.size() << " struct definitions, "
                  << semantic_program.enum_definitions.size() << " enum definitions\n";

        CppEmitResult emitted = emitCpp(semantic_program);
        const std::string output_filename = filename + ".generated.cpp";
        std::ofstream generated_file(output_filename);
        generated_file << emitted.code;
        generated_file.close();

        std::cout << "Generated C++ written to: " << output_filename << "\n";
        std::cout << "Source map entries: " << emitted.source_map.size() << "\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }

    return 0;
}
