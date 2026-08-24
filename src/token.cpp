
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <format>
#include <print>

namespace lexer {

using Terms = std::vector<std::string>;

enum class TokenType {
    Default,
    Separator,
    Delimiter,
    Keyword,
    Primitive,
    Identifier,
    Literal,
    Operator,
    Empty,
    Unknown
};

} // namespace lexer

template <>
struct std::formatter<lexer::TokenType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const lexer::TokenType& type, std::format_context& ctx) const {
        std::string_view name;
        switch (type) {
            case lexer::TokenType::Default: name = "Default"; break;
            case lexer::TokenType::Separator: name = "Separator"; break;
            case lexer::TokenType::Delimiter: name = "Delimiter"; break;
            case lexer::TokenType::Keyword: name = "Keyword"; break;
            case lexer::TokenType::Primitive: name = "Primitive"; break;
            case lexer::TokenType::Identifier: name = "Identifier"; break;
            case lexer::TokenType::Literal: name = "Literal"; break;
            case lexer::TokenType::Empty: name = "Empty"; break;
            case lexer::TokenType::Operator: name = "Operator"; break;
            case lexer::TokenType::Unknown: name = "Unknown"; break;
        }
        return std::format_to(ctx.out(), "{}", name);
    }
};

namespace lexer {

Terms loadTerms(const std::string& filename) {
    std::ifstream termsFile(filename);
    std::vector<std::string> terms{};
    std::string line = "";

    while (std::getline(termsFile, line)) {
        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            continue; // Empty line
        }
        
        line = line.substr(start);
        
        // Skip comment lines
        if (line[0] == '/' || line[0] == '#') {
            continue;
        }
        
        // Remove inline comments (// style)
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim trailing whitespace
        size_t end = line.find_last_not_of(" \t");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        }
        
        if (!line.empty()) {
            terms.push_back(line);
        }
    }

    return terms;
}

bool isSeparator(char c) {
    static const std::string separators = ",;(){}[]<>"; 
    return separators.find(c) != std::string::npos;
}

bool isDelimiter(char c) {
    static const std::string delimiters = " \n\t";
    return delimiters.find(c) != std::string::npos;
}

bool isKeyword(const std::string& str) {
    static const Terms keywords = loadTerms("src/keywords.txt");
    return std::find(keywords.begin(), keywords.end(), str) != keywords.end();
}

bool isPrimitive(const std::string& str) {
    static const Terms primitives = loadTerms("src/primitives.txt");
    return std::find(primitives.begin(), primitives.end(), str) != primitives.end();
}

bool isLiteral(const std::string& str) {
    if (str.size() >= 2 &&
        ((str.front() == '"' && str.back() == '"') ||
         (str.front() == '\'' && str.back() == '\''))) {
        return true;
    }

    bool hasDecimalPoint = false;
    if (str.empty()) {
        return false;
    }

    for (char c : str) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            continue;
        }
        if (c == '.' && !hasDecimalPoint) {
            hasDecimalPoint = true;
            continue;
        }
        return false;
    }

    return true;
}

bool isOperator(const std::string& str) {
    static const Terms operators = loadTerms("src/operators.txt");
    return std::find(operators.begin(), operators.end(), str) != operators.end();
}

struct Token {
    const std::string value;
    TokenType type;

    Token(const std::string& val) : value(val), type(TokenType::Default) {
        findTokenType();
    }

    operator std::string() const { return value; }

    private:

    void findTokenType() {
        if (value.empty()) {
            type = TokenType::Empty;
            return;
        }
       
        if (isSeparator(value[0])) {
            type = TokenType::Separator;
            return;
        }
        
        if (isDelimiter(value[0])) {
            type = TokenType::Delimiter;
            return;
        }

        if (isKeyword(value)) {
            type = TokenType::Keyword;
            return;
        }

        if (isPrimitive(value)) {
            type = TokenType::Primitive;
            return;
        }

        if (isLiteral(value)) {
            type = TokenType::Literal;
            return;
        }

        if (isOperator(value)) {
            type = TokenType::Operator;
            return;
        }

        type = TokenType::Unknown;
    }
};

using Tokens = std::vector<Token>;

}
