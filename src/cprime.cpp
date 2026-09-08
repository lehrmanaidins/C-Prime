
#include <iostream>
#include <iomanip>

#include "lexer/lexer.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string filename = argv[1];

    std::vector<TokenVariant> tokens = lexFile(filename);
    
    for (const TokenVariant& token : tokens) {
        std::cout << toString(token) << std::endl;
    }

    return EXIT_SUCCESS;
}
