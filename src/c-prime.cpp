
#include <string>
#include <vector>
#include <fstream>
#include <print>

#include "lexer/lexer.cpp"
#include "parser/preparser.cpp"

void printPhraseTree(const std::shared_ptr<Phrase>& phrase, int depth = 0) {
    std::println("{}Phrase with {} tokens", std::string(depth * 2, ' '), phrase->tokens.size());
    for (const auto& token : phrase->tokens) {
        std::println("{}  {} : {}", std::string(depth * 2 + 2, ' '), token->type, token->value);
    }

    for (const auto& nested : phrase->nested_phrases) {
        printPhraseTree(nested, depth + 1);
    }
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    const bool exists = file.is_open();
    file.close();
    return exists;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::println("Usage: {} <filename>", argv[0]);
        return 1;
    }

    const std::string filename = argv[1];

    std::println("Lexing file: {}", filename);
    
    if (!fileExists(filename)) {
        std::println("Error: File {} does not exist.", filename);
        return 1;
    }

    Tokens tokens = lexFile(filename);

    std::println("Found {} tokens:", tokens.size());

    for (const auto& token : tokens) {
        std::println("\ttoken type: {}\ttoken:\t{}", token->type, token->value);
    }

    Phrases phrases = preparseTokens(tokens);

    std::println("Found {} phrases:", phrases.size());
    for (const std::shared_ptr<Phrase>& phrase : phrases) {
        printPhraseTree(phrase);
    }

    return 0;
}
