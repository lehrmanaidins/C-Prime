
#include <iostream>
#include <iomanip>

#include "io.cpp"
#include "lexemizer.cpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string filename = argv[1];

    std::vector<lexemizer::Lexeme> lexemes = lexemizer::lexemize(io::readFileLines(filename));

    for (const lexemizer::Lexeme& lexeme : lexemes) {
        std::cout << std::right << std::setw(24) << lexeme.type
                  << " (Line: " << lexeme.line
                  << ", Column: " << lexeme.column
                  << ")\t| " << lexeme.text
                  << std::endl;
    }

    return EXIT_SUCCESS;
}