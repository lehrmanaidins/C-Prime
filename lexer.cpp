
#include <string>
#include <vector>
#include <print>

using Token = std::string;
using Tokens = std::vector<Token>;

Tokens lex(const char* filename) {
    return std::vector<Token>{};
}

/*
 * Lexer for C-Prime source files.
 * @param filename The name of the C-Prime source file to lex.
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::println("Usage: {} <filename>", argv[0]);
        return 1;
    }
    Tokens tokens = lex(argv[1]);

    for (const Token& token : tokens) {
        std::println("{}", token);
    }

    return 0;
}
