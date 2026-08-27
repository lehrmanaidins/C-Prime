#pragma once

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

struct SourceLocation {
    size_t line;
    size_t column;
};

enum class SemanticExpressionKind {
    Identifier,
    Literal,
    Binary,
    Call,
    TupleLiteral,
    InitializerList,
    Raw
};

struct SemanticExpressionIR {
    SemanticExpressionKind kind = SemanticExpressionKind::Raw;
    std::string text;
    std::string operator_symbol;
    std::vector<SemanticExpressionIR> children;
    SourceLocation location{0, 0};
};

static std::string trim(const std::string& text) {
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    if (start == text.size()) {
        return "";
    }

    size_t end = text.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end]))) {
        --end;
    }

    return text.substr(start, end - start + 1);
}

static std::vector<std::string> splitTopLevel(const std::string& text, char delimiter) {
    std::vector<std::string> parts{};
    std::string current;
    int paren_depth = 0;
    int brace_depth = 0;
    int bracket_depth = 0;
    char quote = '\0';
    bool escaped = false;

    for (char ch : text) {
        if (quote != '\0') {
            current.push_back(ch);
            if (ch == '\\' && !escaped) {
                escaped = true;
                continue;
            }
            if (ch == quote && !escaped) {
                quote = '\0';
            }
            escaped = false;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            quote = ch;
            current.push_back(ch);
            continue;
        }

        if (ch == '(') {
            ++paren_depth;
        } else if (ch == ')') {
            --paren_depth;
        } else if (ch == '{') {
            ++brace_depth;
        } else if (ch == '}') {
            --brace_depth;
        } else if (ch == '[') {
            ++bracket_depth;
        } else if (ch == ']') {
            --bracket_depth;
        }

        if (ch == delimiter && paren_depth == 0 && brace_depth == 0 && bracket_depth == 0) {
            parts.push_back(trim(current));
            current.clear();
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        parts.push_back(trim(current));
    }

    return parts;
}

static bool isIntegerLiteral(const std::string& text) {
    if (text.empty()) {
        return false;
    }

    for (char ch : text) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    return true;
}

static bool isFloatLiteral(const std::string& text) {
    bool saw_dot = false;
    bool saw_digit = false;

    if (text.empty()) {
        return false;
    }

    for (char ch : text) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            saw_digit = true;
            continue;
        }

        if (ch == '.' && !saw_dot) {
            saw_dot = true;
            continue;
        }

        return false;
    }

    return saw_dot && saw_digit;
}

static std::optional<std::string> inferLiteralTypeName(const std::string& text) {
    const std::string literal = trim(text);
    if (literal == "true" || literal == "false") {
        return std::optional<std::string>{"bool"};
    }

    if (literal.size() >= 3 && literal.front() == '\'' && literal.back() == '\'') {
        return std::optional<std::string>{"char8"};
    }

    if (literal.size() >= 2 && literal.front() == '"' && literal.back() == '"') {
        return std::optional<std::string>{"char8"};
    }

    if (isIntegerLiteral(literal)) {
        return std::optional<std::string>{"int32"};
    }

    if (isFloatLiteral(literal)) {
        return std::optional<std::string>{"float64"};
    }

    return std::nullopt;
}

static bool isIdentifierText(const std::string& text) {
    if (text.empty()) {
        return false;
    }

    for (char ch : text) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            return false;
        }
    }

    return true;
}

static bool isWrappedByMatchingParens(const std::string& expression) {
    if (expression.size() < 2 || expression.front() != '(' || expression.back() != ')') {
        return false;
    }

    int paren_depth = 0;
    int brace_depth = 0;
    int bracket_depth = 0;
    char quote = '\0';
    bool escaped = false;

    for (size_t i = 0; i < expression.size(); ++i) {
        const char ch = expression[i];
        if (quote != '\0') {
            if (ch == '\\' && !escaped) {
                escaped = true;
                continue;
            }
            if (ch == quote && !escaped) {
                quote = '\0';
            }
            escaped = false;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            quote = ch;
            continue;
        }

        if (ch == '(') {
            ++paren_depth;
        } else if (ch == ')') {
            --paren_depth;
            if (paren_depth == 0 && i + 1 < expression.size()) {
                return false;
            }
        } else if (ch == '{') {
            ++brace_depth;
        } else if (ch == '}') {
            --brace_depth;
        } else if (ch == '[') {
            ++bracket_depth;
        } else if (ch == ']') {
            --bracket_depth;
        }

        if (paren_depth < 0 || brace_depth < 0 || bracket_depth < 0) {
            return false;
        }
    }

    return paren_depth == 0 && brace_depth == 0 && bracket_depth == 0 && quote == '\0';
}

static bool hasTopLevelDelimiter(const std::string& text, char delimiter) {
    return splitTopLevel(text, delimiter).size() > 1;
}

static bool isUnaryOperatorPosition(const std::string& expression, size_t pos) {
    if (pos == 0) {
        return true;
    }

    size_t prev = pos;
    while (prev > 0) {
        --prev;
        if (!std::isspace(static_cast<unsigned char>(expression[prev]))) {
            break;
        }
    }

    const char ch = expression[prev];
    return ch == '(' || ch == '[' || ch == '{' || ch == ',' || ch == '='
        || ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%'
        || ch == '!' || ch == '<' || ch == '>' || ch == '&' || ch == '|';
}

static std::optional<size_t> findTopLevelOperator(const std::string& expression, const std::vector<std::string>& operators) {
    int paren_depth = 0;
    int brace_depth = 0;
    int bracket_depth = 0;
    char quote = '\0';
    bool escaped = false;
    std::optional<size_t> found{};

    for (size_t i = 0; i < expression.size(); ++i) {
        const char ch = expression[i];
        if (quote != '\0') {
            if (ch == '\\' && !escaped) {
                escaped = true;
                continue;
            }
            if (ch == quote && !escaped) {
                quote = '\0';
            }
            escaped = false;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            quote = ch;
            continue;
        }

        if (ch == '(') {
            ++paren_depth;
            continue;
        }
        if (ch == ')') {
            if (paren_depth > 0) --paren_depth;
            continue;
        }
        if (ch == '{') {
            ++brace_depth;
            continue;
        }
        if (ch == '}') {
            if (brace_depth > 0) --brace_depth;
            continue;
        }
        if (ch == '[') {
            ++bracket_depth;
            continue;
        }
        if (ch == ']') {
            if (bracket_depth > 0) --bracket_depth;
            continue;
        }

        if (paren_depth != 0 || brace_depth != 0 || bracket_depth != 0) {
            continue;
        }

        for (const auto& op : operators) {
            if (expression.compare(i, op.size(), op) != 0) {
                continue;
            }

            if ((op == "+" || op == "-") && isUnaryOperatorPosition(expression, i)) {
                continue;
            }

            found = i;
        }
    }

    return found;
}

static std::string findOperatorAt(const std::string& expression, size_t pos, const std::vector<std::string>& operators) {
    for (const auto& op : operators) {
        if (expression.compare(pos, op.size(), op) == 0) {
            return op;
        }
    }

    return "";
}

static std::optional<size_t> findCallOpenParen(const std::string& expression) {
    char quote = '\0';
    bool escaped = false;
    for (size_t i = 0; i < expression.size(); ++i) {
        const char ch = expression[i];
        if (quote != '\0') {
            if (ch == '\\' && !escaped) {
                escaped = true;
                continue;
            }
            if (ch == quote && !escaped) {
                quote = '\0';
            }
            escaped = false;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            quote = ch;
            continue;
        }

        if (ch == '(') {
            return i;
        }
    }

    return std::nullopt;
}

static SemanticExpressionIR parseExpressionIR(const std::string& raw_expression, SourceLocation location) {
    const std::string expression = trim(raw_expression);

    SemanticExpressionIR expr{};
    expr.text = expression;
    expr.location = location;

    if (expression.empty()) {
        return expr;
    }

    for (const std::string op : {"+=", "-=", "*=", "/=", "%=", "**=", "&=", "|=", "^=", "~=", "<<=", ">>="}) {
        if (expression.find(op) != std::string::npos) {
            expr.kind = SemanticExpressionKind::Raw;
            return expr;
        }
    }

    if (inferLiteralTypeName(expression).has_value()) {
        expr.kind = SemanticExpressionKind::Literal;
        return expr;
    }

    if (isWrappedByMatchingParens(expression)) {
        const std::string inner = trim(expression.substr(1, expression.size() - 2));
        if (!hasTopLevelDelimiter(inner, ',')) {
            return parseExpressionIR(inner, location);
        }
    }

    if (expression.front() == '{' && expression.back() == '}') {
        expr.kind = SemanticExpressionKind::InitializerList;
        const std::string inner = trim(expression.substr(1, expression.size() - 2));
        for (const auto& part : splitTopLevel(inner, ',')) {
            expr.children.push_back(parseExpressionIR(part, location));
        }
        return expr;
    }

    if (expression.front() == '(' && expression.back() == ')' && hasTopLevelDelimiter(expression.substr(1, expression.size() - 2), ',')) {
        expr.kind = SemanticExpressionKind::TupleLiteral;
        const std::string inner = trim(expression.substr(1, expression.size() - 2));
        for (const auto& part : splitTopLevel(inner, ',')) {
            expr.children.push_back(parseExpressionIR(part, location));
        }
        return expr;
    }

    const std::vector<std::vector<std::string>> precedence_groups = {
        {"||"},
        {"&&"},
        {"==", "!="},
        {">=", "<=", ">", "<"},
        {"+", "-"},
        {"*", "/", "%"}
    };

    for (const auto& operators : precedence_groups) {
        const std::optional<size_t> pos = findTopLevelOperator(expression, operators);
        if (pos.has_value()) {
            const std::string op = findOperatorAt(expression, pos.value(), operators);
            expr.kind = SemanticExpressionKind::Binary;
            expr.operator_symbol = op;
            expr.children.push_back(parseExpressionIR(expression.substr(0, pos.value()), location));
            expr.children.push_back(parseExpressionIR(expression.substr(pos.value() + op.size()), location));
            return expr;
        }
    }

    const std::optional<size_t> open_paren = findCallOpenParen(expression);
    if (open_paren.has_value() && expression.back() == ')' && open_paren.value() > 0) {
        const std::string callee = trim(expression.substr(0, open_paren.value()));
        if (!isIdentifierText(callee)) {
            expr.kind = SemanticExpressionKind::Raw;
            return expr;
        }

        expr.kind = SemanticExpressionKind::Call;
        expr.operator_symbol = callee;
        const std::string args = trim(expression.substr(open_paren.value() + 1, expression.size() - open_paren.value() - 2));
        for (const auto& part : splitTopLevel(args, ',')) {
            if (!part.empty()) {
                expr.children.push_back(parseExpressionIR(part, location));
            }
        }
        return expr;
    }

    if (isIdentifierText(expression)) {
        expr.kind = SemanticExpressionKind::Identifier;
        return expr;
    }

    expr.kind = SemanticExpressionKind::Raw;
    return expr;
}