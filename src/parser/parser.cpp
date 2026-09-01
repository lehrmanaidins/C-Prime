
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "preparser.cpp"
#include "parsed_phrase.cpp"
#include "phrases/assignment.cpp"
#include "phrases/control_flow.cpp"
#include "phrases/enum_definition.cpp"
#include "phrases/enum_value_definition.cpp"
#include "phrases/function.cpp"
#include "phrases/import.cpp"
#include "phrases/parameter_declaration.cpp"
#include "phrases/parameter_definition.cpp"
#include "phrases/return_statement.cpp"
#include "phrases/statement.cpp"
#include "phrases/struct_definition.cpp"
#include "phrases/type_definition.cpp"
#include "phrases/union_definition.cpp"
#include "phrases/unsafe_block.cpp"
#include "phrases/variable_declaration.cpp"

static std::string trimParserText(const std::string& text) {
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

static bool findParentheses(const Tokens& tokens, size_t& open_index, size_t& close_index) {
    open_index = tokens.size();
    close_index = tokens.size();

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (!tokens[i]) {
            continue;
        }

        if (tokens[i]->value == "(") {
            open_index = i;
            break;
        }
    }

    if (open_index == tokens.size()) {
        return false;
    }

    for (size_t i = open_index + 1; i < tokens.size(); ++i) {
        if (!tokens[i]) {
            continue;
        }

        if (tokens[i]->value == ")") {
            close_index = i;
            return true;
        }
    }

    return false;
}

static std::string joinTokenRange(const Tokens& tokens, size_t begin, size_t end_exclusive) {
    if (begin >= end_exclusive || begin >= tokens.size()) {
        return "";
    }

    std::string joined;
    for (size_t i = begin; i < end_exclusive && i < tokens.size(); ++i) {
        if (!tokens[i]) {
            continue;
        }

        if (!joined.empty()) {
            joined += " ";
        }
        joined += tokens[i]->value;
    }

    return joined;
}

static bool hasTokenValue(const Tokens& tokens, const std::string& value) {
    for (const auto& token : tokens) {
        if (token && token->value == value) {
            return true;
        }
    }
    return false;
}

static bool isDeclarationStarter(const std::string& value) {
    return value == "mutable"
        || value == "const"
        || value == "unsafe"
        || value == "primitive"
        || value == "type"
        || value == "struct"
        || value == "enum"
        || value == "union"
        || value == "function"
        || value == "import";
}

static bool looksLikeIdentifier(const std::string& value) {
    if (value.empty() || !(std::isalpha(static_cast<unsigned char>(value.front())) || value.front() == '_')) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](char ch) {
        return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
    });
}

static std::string findMissingSemicolonBeforeToken(const Tokens& tokens) {
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    int angle_depth = 0;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (!tokens[i]) {
            continue;
        }

        const std::string value = tokens[i]->value;
        if (value == "(") {
            ++paren_depth;
        } else if (value == ")") {
            if (paren_depth > 0) {
                --paren_depth;
            }
        } else if (value == "[") {
            ++bracket_depth;
        } else if (value == "]") {
            if (bracket_depth > 0) {
                --bracket_depth;
            }
        } else if (value == "{") {
            ++brace_depth;
        } else if (value == "}") {
            if (brace_depth > 0) {
                --brace_depth;
            }
        } else if (value == "<"
            && (angle_depth > 0 || (i > 0 && tokens[i - 1] && looksLikeIdentifier(tokens[i - 1]->value)))) {
            ++angle_depth;
        } else if (value == ">" && angle_depth > 0) {
            --angle_depth;
        }

        if (i == 0 || paren_depth > 0 || bracket_depth > 0 || brace_depth > 0 || angle_depth > 0) {
            continue;
        }

        if (!isDeclarationStarter(value)) {
            continue;
        }

        if (!tokens[i - 1]) {
            continue;
        }

        const std::string prev = tokens[i - 1]->value;
        if (isDeclarationStarter(prev) || prev == ":" || prev == "," || prev == "|") {
            continue;
        }

        return value;
    }

    return "";
}

static std::vector<std::string> collectLeadingFunctionTags(const Tokens& tokens) {
    std::vector<std::string> tags{};
    size_t index = 0;
    while (index + 1 < tokens.size() && tokens[index] && tokens[index]->value == "@" && tokens[index + 1]) {
        tags.push_back(tokens[index + 1]->value);
        index += 2;
    }
    return tags;
}

static size_t functionPrefixStart(const Tokens& tokens) {
    size_t index = 0;
    while (index + 1 < tokens.size() && tokens[index] && tokens[index]->value == "@" && tokens[index + 1]) {
        index += 2;
    }
    return index;
}

static void validatePhrase(const std::shared_ptr<Phrase>& phrase, ParsedPhraseKind parent_kind) {
    if (!phrase) {
        return;
    }

    const size_t start = functionPrefixStart(phrase->tokens);
    if (start < phrase->tokens.size() && phrase->tokens[start]
        && (phrase->tokens[start]->value == "function" || phrase->tokens[start]->value == "template"
            || (phrase->tokens[start]->value == "unsafe" && phrase->tokens.size() > start + 1 && phrase->tokens[start + 1]
                && phrase->tokens[start + 1]->value == "function"))) {
        return;
    }

    const std::string semicolon_break = findMissingSemicolonBeforeToken(phrase->tokens);
    if (!semicolon_break.empty()) {
        throw std::runtime_error(
            "Parser error: missing semicolon before '" + semicolon_break + "' in phrase: "
            + joinTokenRange(phrase->tokens, 0, phrase->tokens.size())
        );
    }

}

static size_t findTokenIndex(const Tokens& tokens, const std::string& value) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] && tokens[i]->value == value) {
            return i;
        }
    }
    return tokens.size();
}

static size_t findTokenIndexFrom(const Tokens& tokens, const std::string& value, size_t from) {
    for (size_t i = from; i < tokens.size(); ++i) {
        if (tokens[i] && tokens[i]->value == value) {
            return i;
        }
    }
    return tokens.size();
}

// If the phrase begins with a `template < ... >` prefix, returns the index of
// the token immediately after the matching `>` (where the `struct` / `type`
// keyword is expected). Returns 0 when there is no template prefix.
static size_t templatePrefixEnd(const Tokens& tokens) {
    if (tokens.size() < 4 || !tokens[0] || tokens[0]->value != "template" || !tokens[1] || tokens[1]->value != "<") {
        return 0;
    }

    size_t angle_depth = 0;
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (!tokens[i]) {
            continue;
        }
        if (tokens[i]->value == "<") {
            ++angle_depth;
        } else if (tokens[i]->value == ">") {
            --angle_depth;
            if (angle_depth == 0) {
                return i + 1;
            }
        }
    }

    return 0;
}

static bool isAssignmentOperator(const std::string& value) {
    return value == "=" || value == "+=" || value == "-=" || value == "*=" || value == "/="
        || value == "%=" || value == "**=" || value == "&=" || value == "|=" || value == "^="
        || value == "~=" || value == "<<=" || value == ">>=";
}

static size_t findAssignmentOperatorIndex(const Tokens& tokens) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] && isAssignmentOperator(tokens[i]->value)) {
            return i;
        }
    }

    return tokens.size();
}

static bool isFunctionPhrase(const std::shared_ptr<Phrase>& phrase, size_t& open_paren, size_t& close_paren) {
    if (!phrase || phrase->tokens.empty()) {
        return false;
    }

    const size_t start = functionPrefixStart(phrase->tokens);
    if (start >= phrase->tokens.size()) {
        return false;
    }

    const bool has_template_prefix = phrase->tokens[start] && phrase->tokens[start]->value == "template";
    const bool has_unsafe_prefix = phrase->tokens[start] && phrase->tokens[start]->value == "unsafe"
        && phrase->tokens.size() > start + 1 && phrase->tokens[start + 1] && phrase->tokens[start + 1]->value == "function";
    const size_t function_index = findTokenIndex(phrase->tokens, "function");
    if ((has_template_prefix || has_unsafe_prefix || (function_index == start)) && function_index != phrase->tokens.size()) {
        return findParentheses(phrase->tokens, open_paren, close_paren) && open_paren > function_index;
    }

    return false;
}

static bool isTemplateTypeCategory(const std::string& name) {
    return name == "Integral" || name == "Floating";
}

static std::string joinTemplateConstraintTokens(const Tokens& tokens, size_t begin, size_t end_exclusive) {
    return joinTokenRange(tokens, begin, end_exclusive);
}

static std::vector<ParsedTemplateParameter> parseTemplateParameters(const Tokens& tokens, size_t function_index) {
    const size_t start = functionPrefixStart(tokens);
    if (function_index <= start || start >= tokens.size() || !tokens[start] || tokens[start]->value != "template") {
        return {};
    }

    if (function_index < start + 5
        || !tokens[start + 1] || tokens[start + 1]->value != "<"
        || !tokens[function_index - 1] || tokens[function_index - 1]->value != ">") {
        throw std::runtime_error("Parser error: invalid template declaration");
    }

    std::vector<ParsedTemplateParameter> parameters;
    size_t index = start + 2;
    while (index < function_index - 1) {
        if (!tokens[index] || tokens[index]->value != "type" || index + 1 >= function_index - 1 || !tokens[index + 1]) {
            throw std::runtime_error("Parser error: template parameters must use 'type Name'");
        }

        ParsedTemplateParameter parameter{};
        parameter.name = tokens[index + 1]->value;
        const bool duplicate_name = std::any_of(parameters.begin(), parameters.end(), [&](const ParsedTemplateParameter& existing) {
            return existing.name == parameter.name;
        });
        if (parameter.name.empty() || duplicate_name) {
            throw std::runtime_error("Parser error: duplicate or empty template parameter '" + parameter.name + "'");
        }
        index += 2;

        if (index < function_index - 1 && tokens[index] && tokens[index]->value == ":") {
            ++index;
            while (index < function_index - 1) {
                if (!tokens[index]) {
                    throw std::runtime_error("Parser error: invalid template type constraint");
                }

                const size_t constraint_begin = index;
                while (index < function_index - 1 && tokens[index]
                    && tokens[index]->value != "|" && tokens[index]->value != ",") {
                    ++index;
                }

                const std::string constraint = joinTemplateConstraintTokens(tokens, constraint_begin, index);
                if (constraint.empty()) {
                    throw std::runtime_error("Parser error: invalid template type constraint");
                }

                if (isTemplateTypeCategory(constraint)) {
                    if (!parameter.allowed_types.empty() || !parameter.category.empty()) {
                        throw std::runtime_error("Parser error: template category cannot be combined with other constraints");
                    }
                    parameter.category = constraint;
                } else {
                    if (!parameter.category.empty()) {
                        throw std::runtime_error("Parser error: template category cannot be combined with other constraints");
                    }
                    parameter.allowed_types.push_back(constraint);
                }

                if (index >= function_index - 1 || !tokens[index] || tokens[index]->value != "|") {
                    break;
                }
                ++index;
            }

            if (parameter.allowed_types.empty() && parameter.category.empty()) {
                throw std::runtime_error("Parser error: template constraint cannot be empty");
            }
        }

        parameters.push_back(parameter);
        if (index < function_index - 1) {
            if (!tokens[index] || tokens[index]->value != ",") {
                throw std::runtime_error("Parser error: expected ',' between template parameters");
            }
            ++index;
        }
    }

    if (parameters.empty()) {
        throw std::runtime_error("Parser error: template declaration needs at least one type parameter");
    }

    return parameters;
}

static bool isTypeDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase) {
        return false;
    }
    const size_t keyword = templatePrefixEnd(phrase->tokens);
    return phrase->tokens.size() >= keyword + 4 && phrase->tokens[keyword]
        && phrase->tokens[keyword]->value == "type"
        && findTokenIndexFrom(phrase->tokens, ":", keyword + 2) != phrase->tokens.size();
}

static bool isStructDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase) {
        return false;
    }
    const size_t keyword = templatePrefixEnd(phrase->tokens);
    return phrase->tokens.size() >= keyword + 2 && phrase->tokens[keyword]
        && phrase->tokens[keyword]->value == "struct";
}

static bool isEnumDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() >= 2 && phrase->tokens[0] && phrase->tokens[0]->value == "enum";
}

static bool isUnionDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() >= 4 && phrase->tokens[0] && phrase->tokens[0]->value == "union" && hasTokenValue(phrase->tokens, ":");
}

static std::vector<std::string> parseUnionMembers(const Tokens& tokens, size_t colon_index) {
    std::vector<std::string> members;
    size_t member_begin = colon_index + 1;
    for (size_t i = colon_index + 1; i <= tokens.size(); ++i) {
        if (i == tokens.size() || (tokens[i] && tokens[i]->value == "|")) {
            const std::string member = joinTokenRange(tokens, member_begin, i);
            if (!member.empty()) {
                members.push_back(member);
            }
            member_begin = i + 1;
        }
    }
    return members;
}

static bool isVariableDeclarationPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 2) {
        return false;
    }

    if (phrase->tokens[0] && (phrase->tokens[0]->value == "type" || phrase->tokens[0]->value == "function" || phrase->tokens[0]->value == "return")) {
        return false;
    }

    if (phrase->tokens[0] && (phrase->tokens[0]->value == "struct" || phrase->tokens[0]->value == "enum")) {
        return false;
    }

    if (phrase->tokens[0] && (phrase->tokens[0]->value == "if" || phrase->tokens[0]->value == "else" || phrase->tokens[0]->value == "while" || phrase->tokens[0]->value == "for")) {
        return false;
    }

    if (phrase->tokens[0] && (phrase->tokens[0]->value == "requires" || phrase->tokens[0]->value == "ensures")) {
        return false;
    }

    if (hasTokenValue(phrase->tokens, ":")) {
        return false;
    }

    const size_t equal_index = findAssignmentOperatorIndex(phrase->tokens);
    if (equal_index != phrase->tokens.size()) {
        return equal_index >= 2;
    }

    if (hasTokenValue(phrase->tokens, "(") || hasTokenValue(phrase->tokens, ")")
        || hasTokenValue(phrase->tokens, "{") || hasTokenValue(phrase->tokens, "}")) {
        return false;
    }

    const auto& last = phrase->tokens.back();
    if (!last || last->type != TokenCatagory::Identifier) {
        return false;
    }

    return true;
}

static bool isReturnPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && !phrase->tokens.empty() && phrase->tokens[0] && phrase->tokens[0]->value == "return";
}

static bool isBreakPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() == 1 && phrase->tokens[0] && phrase->tokens[0]->value == "break";
}

static bool isContinuePhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() == 1 && phrase->tokens[0] && phrase->tokens[0]->value == "continue";
}

static bool isKeywordPhrase(const std::shared_ptr<Phrase>& phrase, const std::string& keyword) {
    return phrase && !phrase->tokens.empty() && phrase->tokens[0] && phrase->tokens[0]->value == keyword;
}

static bool hasSuffix(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size()
        && value.substr(value.size() - suffix.size()) == suffix;
}

static std::string unquoteImportPath(const std::string& raw_path) {
    const std::string path = trimParserText(raw_path);
    if (path.size() >= 2
        && ((path.front() == '"' && path.back() == '"')
            || (path.front() == '\'' && path.back() == '\''))) {
        return path.substr(1, path.size() - 2);
    }

    return path;
}

static ParsedImportKind detectImportKind(const std::string& path) {
    if (hasSuffix(path, ".cprime")) {
        return ParsedImportKind::CPrime;
    }

    return ParsedImportKind::Cpp;
}

static bool isAssignmentPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 3) {
        return false;
    }

    const size_t equal_index = findAssignmentOperatorIndex(phrase->tokens);
    if (equal_index == phrase->tokens.size() || equal_index == 0) {
        return false;
    }

    if (phrase->tokens[0] && phrase->tokens[0]->value == "mutable") {
        return false;
    }

    if (hasTokenValue(phrase->tokens, "primitive")) {
        return false;
    }

    return equal_index == 1;
}

static bool isCallPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 3 || !phrase->tokens[0]) {
        return false;
    }

    if (phrase->tokens[0]->type != TokenCatagory::Identifier) {
        return false;
    }

    size_t open_paren = phrase->tokens.size();
    size_t close_paren = phrase->tokens.size();
    if (!findParentheses(phrase->tokens, open_paren, close_paren)) {
        return false;
    }

    if (open_paren == 0 || open_paren % 2 == 0 || close_paren != phrase->tokens.size() - 1) {
        return false;
    }

    for (size_t i = 0; i < open_paren; ++i) {
        if (!phrase->tokens[i]) {
            return false;
        }
        if (i % 2 == 0) {
            if (phrase->tokens[i]->type != TokenCatagory::Identifier) {
                return false;
            }
        } else if (phrase->tokens[i]->value != ".") {
            return false;
        }
    }

    return true;
}

static std::string joinDottedTokenRange(const Tokens& tokens, size_t begin, size_t end_exclusive) {
    std::string joined;
    for (size_t i = begin; i < end_exclusive && i < tokens.size(); ++i) {
        if (tokens[i]) {
            joined += tokens[i]->value;
        }
    }
    return joined;
}

static std::vector<std::string> parseCallArguments(const Tokens& tokens, size_t open_paren, size_t close_paren) {
    std::vector<std::string> arguments{};
    std::string current{};
    int nested_paren_depth = 0;

    for (size_t i = open_paren + 1; i < close_paren; ++i) {
        if (!tokens[i]) {
            continue;
        }

        if (tokens[i]->value == "(") {
            ++nested_paren_depth;
        } else if (tokens[i]->value == ")") {
            if (nested_paren_depth > 0) {
                --nested_paren_depth;
            }
        }

        if (tokens[i]->value == "," && nested_paren_depth == 0) {
            const std::string trimmed = trimParserText(current);
            if (!trimmed.empty()) {
                arguments.push_back(trimmed);
            }
            current.clear();
            continue;
        }

        if (!current.empty()) {
            current += " ";
        }
        current += tokens[i]->value;
    }

    const std::string trimmed = trimParserText(current);
    if (!trimmed.empty()) {
        arguments.push_back(trimmed);
    }

    return arguments;
}

static bool isStructParameterDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 2) {
        return false;
    }

    if (hasTokenValue(phrase->tokens, "=") || hasTokenValue(phrase->tokens, "(") || hasTokenValue(phrase->tokens, ")")) {
        return false;
    }

    return true;
}

static bool isEnumValueDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.empty()) {
        return false;
    }

    if (hasTokenValue(phrase->tokens, "=") || hasTokenValue(phrase->tokens, "(") || hasTokenValue(phrase->tokens, ")")) {
        return false;
    }

    return hasTokenValue(phrase->tokens, ",") || phrase->tokens.size() == 1;
}

static std::vector<std::string> parseEnumValues(const Tokens& tokens) {
    std::vector<std::string> values{};
    std::string current;

    auto flushCurrent = [&]() {
        if (!current.empty()) {
            values.push_back(current);
            current.clear();
        }
    };

    for (const auto& token : tokens) {
        if (!token) {
            continue;
        }

        if (token->value == ",") {
            flushCurrent();
            continue;
        }

        if (!current.empty()) {
            current += " ";
        }
        current += token->value;
    }

    flushCurrent();
    return values;
}

static void parseFunctionParameters(
    const Tokens& tokens,
    size_t open_paren,
    size_t close_paren,
    const std::shared_ptr<Phrase>& source_phrase,
    const std::shared_ptr<ParsedFunction>& parsed_function
) {
    size_t segment_start = open_paren + 1;

    auto flush_segment = [&](size_t segment_end) {
        if (segment_end <= segment_start) {
            return;
        }

        size_t last = segment_end;
        while (last > segment_start && (!tokens[last - 1] || tokens[last - 1]->value.empty())) {
            --last;
        }

        if (last <= segment_start) {
            return;
        }

        if (last == segment_start + 1 && tokens[segment_start] && tokens[segment_start]->value == "void") {
            return;
        }

        std::string param_name = tokens[last - 1] ? tokens[last - 1]->value : "";
        std::string type_name = joinTokenRange(tokens, segment_start, last - 1);

        auto parameter = std::make_shared<ParsedParameterDeclaration>(type_name, param_name, source_phrase);
        parsed_function->parameters.push_back(parameter);
        parsed_function->addNested(parameter);
    };

    for (size_t i = open_paren + 1; i < close_paren; ++i) {
        if (!tokens[i]) {
            continue;
        }

        if (tokens[i]->value == ",") {
            flush_segment(i);
            segment_start = i + 1;
        }
    }

    flush_segment(close_paren);
}

static std::string parseFunctionReturnType(const Tokens& tokens, size_t close_paren) {
    if (close_paren + 1 >= tokens.size() || !tokens[close_paren + 1] || tokens[close_paren + 1]->value != "->") {
        return "";
    }

    size_t return_end = close_paren + 2;
    while (return_end < tokens.size() && tokens[return_end]
        && tokens[return_end]->value != ":"
        && tokens[return_end]->value != "requires"
        && tokens[return_end]->value != "ensures") {
        ++return_end;
    }

    if (return_end > close_paren + 3 && tokens[return_end - 1]
        && tokens[return_end - 1]->type == TokenCatagory::Identifier) {
        --return_end;
    }

    return joinTokenRange(tokens, close_paren + 2, return_end);
}

static std::string parseFunctionReturnValueName(const Tokens& tokens, size_t close_paren) {
    if (close_paren + 1 >= tokens.size() || !tokens[close_paren + 1] || tokens[close_paren + 1]->value != "->") {
        return "";
    }

    size_t return_end = close_paren + 2;
    while (return_end < tokens.size() && tokens[return_end]
        && tokens[return_end]->value != ":"
        && tokens[return_end]->value != "requires"
        && tokens[return_end]->value != "ensures") {
        ++return_end;
    }

    if (return_end > close_paren + 3 && tokens[return_end - 1]
        && tokens[return_end - 1]->type == TokenCatagory::Identifier) {
        return tokens[return_end - 1]->value;
    }

    return "";
}

static std::string parseFunctionContractClause(const Tokens& tokens, const std::string& keyword) {
    const size_t start = findTokenIndex(tokens, keyword);
    if (start == tokens.size()) {
        return "";
    }

    size_t end = start + 1;
    while (end < tokens.size() && tokens[end] && tokens[end]->value != ";") {
        ++end;
    }

    return joinTokenRange(tokens, start + 1, end);
}

static std::string parseParenthesizedContent(const Tokens& tokens) {
    size_t open_paren = tokens.size();
    size_t close_paren = tokens.size();
    if (!findParentheses(tokens, open_paren, close_paren)) {
        return "";
    }

    return joinTokenRange(tokens, open_paren + 1, close_paren);
}

static std::vector<std::string> splitParserTopLevel(const std::string& text, char delimiter) {
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
            if (paren_depth > 0) --paren_depth;
        } else if (ch == '{') {
            ++brace_depth;
        } else if (ch == '}') {
            if (brace_depth > 0) --brace_depth;
        } else if (ch == '[') {
            ++bracket_depth;
        } else if (ch == ']') {
            if (bracket_depth > 0) --bracket_depth;
        }

        if (ch == delimiter && paren_depth == 0 && brace_depth == 0 && bracket_depth == 0) {
            parts.push_back(trimParserText(current));
            current.clear();
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty()) {
        parts.push_back(trimParserText(current));
    }

    return parts;
}

static ParsedForStatement parseForStatement(const std::shared_ptr<Phrase>& phrase) {
    const auto parts = splitParserTopLevel(parseParenthesizedContent(phrase->tokens), ';');
    const std::string initializer = parts.size() > 0 ? parts[0] : "";
    const std::string condition = parts.size() > 1 ? parts[1] : "";
    const std::string update = parts.size() > 2 ? parts[2] : "";

    return ParsedForStatement(initializer, condition, update, phrase);
}

std::shared_ptr<ParsedPhrase> parsePhrase(const std::shared_ptr<Phrase>& phrase, ParsedPhraseKind parent_kind = ParsedPhraseKind::Unknown) {
    if (!phrase) {
        return std::make_shared<ParsedPhrase>();
    }

    validatePhrase(phrase, parent_kind);

    std::shared_ptr<ParsedPhrase> parsed_phrase;
    size_t open_paren = 0;
    size_t close_paren = 0;

    if (parent_kind == ParsedPhraseKind::StructDefinition && isStructParameterDefinitionPhrase(phrase)) {
        const std::string parameter_name = phrase->tokens.back() ? phrase->tokens.back()->value : "";
        const std::string parameter_type = phrase->tokens.size() > 1
            ? joinTokenRange(phrase->tokens, 0, phrase->tokens.size() - 1)
            : "";
        parsed_phrase = std::make_shared<ParsedParameterDefinition>(parameter_type, parameter_name, phrase);
    } else if (parent_kind == ParsedPhraseKind::EnumDefinition && isEnumValueDefinitionPhrase(phrase)) {
        parsed_phrase = std::make_shared<ParsedEnumValueDefinition>(parseEnumValues(phrase->tokens), phrase);
    } else if (isFunctionPhrase(phrase, open_paren, close_paren) && open_paren > 1 && phrase->tokens[open_paren - 1]) {
        const std::string function_name = phrase->tokens[open_paren - 1]->value;
        const std::string return_type = parseFunctionReturnType(phrase->tokens, close_paren);
        const size_t function_index = findTokenIndex(phrase->tokens, "function");
        auto function = std::make_shared<ParsedFunction>(function_name, return_type, parseTemplateParameters(phrase->tokens, function_index), phrase);
        function->return_value_name = parseFunctionReturnValueName(phrase->tokens, close_paren);
        function->requires_clause = parseFunctionContractClause(phrase->tokens, "requires");
        function->ensures_clause = parseFunctionContractClause(phrase->tokens, "ensures");
        const size_t prefix_start = functionPrefixStart(phrase->tokens);
        function->tags = collectLeadingFunctionTags(phrase->tokens);
        function->is_unsafe = prefix_start < phrase->tokens.size() && phrase->tokens[prefix_start] && phrase->tokens[prefix_start]->value == "unsafe";
        parseFunctionParameters(phrase->tokens, open_paren, close_paren, phrase, function);
        parsed_phrase = function;
    } else if (isKeywordPhrase(phrase, "unsafe") && phrase->tokens.size() == 1) {
        parsed_phrase = std::make_shared<ParsedUnsafeBlock>(phrase);
    } else if (isKeywordPhrase(phrase, "requires")) {
        parsed_phrase = std::make_shared<ParsedRequiresClause>(joinTokenRange(phrase->tokens, 1, phrase->tokens.size()), phrase);
    } else if (isKeywordPhrase(phrase, "ensures")) {
        parsed_phrase = std::make_shared<ParsedEnsuresClause>(joinTokenRange(phrase->tokens, 1, phrase->tokens.size()), phrase);
    } else if (isTypeDefinitionPhrase(phrase)) {
        const size_t keyword = templatePrefixEnd(phrase->tokens);
        const std::string type_name = phrase->tokens[keyword + 1] ? phrase->tokens[keyword + 1]->value : "";
        const size_t colon_index = findTokenIndexFrom(phrase->tokens, ":", keyword + 2);
        const std::string base_type = joinTokenRange(phrase->tokens, colon_index + 1, phrase->tokens.size());
        auto type_definition = std::make_shared<ParsedTypeDefinition>(type_name, base_type, phrase);
        type_definition->template_parameters = parseTemplateParameters(phrase->tokens, keyword);
        parsed_phrase = type_definition;
    } else if (isStructDefinitionPhrase(phrase)) {
        const size_t keyword = templatePrefixEnd(phrase->tokens);
        const std::string struct_name = phrase->tokens[keyword + 1] ? phrase->tokens[keyword + 1]->value : "";
        auto struct_definition = std::make_shared<ParsedStructDefinition>(struct_name, phrase);
        struct_definition->template_parameters = parseTemplateParameters(phrase->tokens, keyword);
        parsed_phrase = struct_definition;
    } else if (isEnumDefinitionPhrase(phrase)) {
        const std::string enum_name = phrase->tokens[1] ? phrase->tokens[1]->value : "";
        parsed_phrase = std::make_shared<ParsedEnumDefinition>(enum_name, phrase);
    } else if (isUnionDefinitionPhrase(phrase)) {
        const std::string union_name = phrase->tokens[1] ? phrase->tokens[1]->value : "";
        const size_t colon_index = findTokenIndex(phrase->tokens, ":");
        parsed_phrase = std::make_shared<ParsedUnionDefinition>(union_name, parseUnionMembers(phrase->tokens, colon_index), phrase);
    } else if (isKeywordPhrase(phrase, "if")) {
        parsed_phrase = std::make_shared<ParsedIfStatement>(parseParenthesizedContent(phrase->tokens), phrase);
    } else if (isKeywordPhrase(phrase, "while")) {
        parsed_phrase = std::make_shared<ParsedWhileStatement>(parseParenthesizedContent(phrase->tokens), phrase);
    } else if (isKeywordPhrase(phrase, "for")) {
        parsed_phrase = std::make_shared<ParsedForStatement>(parseForStatement(phrase));
    } else if (isKeywordPhrase(phrase, "else")) {
        const bool is_else_if = phrase->tokens.size() > 1 && phrase->tokens[1] && phrase->tokens[1]->value == "if";
        parsed_phrase = std::make_shared<ParsedElseStatement>(is_else_if ? parseParenthesizedContent(phrase->tokens) : "", phrase);
    } else if (isKeywordPhrase(phrase, "import")) {
        const std::string import_path = phrase->tokens.size() > 1 && phrase->tokens[1]
            ? unquoteImportPath(phrase->tokens[1]->value)
            : "";
        parsed_phrase = std::make_shared<ParsedImportStatement>(import_path, detectImportKind(import_path), phrase);
    } else if (isVariableDeclarationPhrase(phrase)) {
        const size_t equal_index = findAssignmentOperatorIndex(phrase->tokens);
        bool is_mutable = false;
        bool is_unsafe = false;
        size_t type_begin = 0;
        while (type_begin < phrase->tokens.size() && phrase->tokens[type_begin]) {
            const std::string& prefix_value = phrase->tokens[type_begin]->value;
            if (prefix_value == "mutable" && !is_mutable) {
                is_mutable = true;
            } else if (prefix_value == "unsafe" && !is_unsafe) {
                is_unsafe = true;
            } else if (prefix_value == "const") {
                // const-by-default; consumed without a dedicated flag
            } else {
                break;
            }
            ++type_begin;
        }
        const bool has_initializer = equal_index != phrase->tokens.size();
        const size_t name_index = has_initializer
            ? (equal_index > 0 ? equal_index - 1 : 0)
            : (phrase->tokens.size() - 1);
        const std::string name = phrase->tokens[name_index] ? phrase->tokens[name_index]->value : "";
        const std::string type_name = joinTokenRange(phrase->tokens, type_begin, name_index);
        const std::string initializer = has_initializer
            ? joinTokenRange(phrase->tokens, equal_index + 1, phrase->tokens.size())
            : "";
        parsed_phrase = std::make_shared<ParsedVariableDeclaration>(type_name, name, initializer, is_mutable, is_unsafe, phrase);
    } else if (isReturnPhrase(phrase)) {
        parsed_phrase = std::make_shared<ParsedReturnStatement>(joinTokenRange(phrase->tokens, 1, phrase->tokens.size()), phrase);
    } else if (isBreakPhrase(phrase)) {
        parsed_phrase = std::make_shared<ParsedBreakStatement>(phrase);
    } else if (isContinuePhrase(phrase)) {
        parsed_phrase = std::make_shared<ParsedContinueStatement>(phrase);
    } else if (isAssignmentPhrase(phrase)) {
        const size_t equal_index = findAssignmentOperatorIndex(phrase->tokens);
        const std::string left = joinTokenRange(phrase->tokens, 0, equal_index);
        const std::string right = joinTokenRange(phrase->tokens, equal_index + 1, phrase->tokens.size());
        const std::string operator_symbol = phrase->tokens[equal_index] ? phrase->tokens[equal_index]->value : "=";
        parsed_phrase = std::make_shared<ParsedAssignment>(left, right, operator_symbol, phrase);
    } else if (isCallPhrase(phrase)) {
        size_t open_paren = phrase->tokens.size();
        size_t close_paren = phrase->tokens.size();

        for (size_t i = 0; i < phrase->tokens.size(); ++i) {
            if (!phrase->tokens[i]) {
                continue;
            }

            if (phrase->tokens[i]->value == "(") {
                open_paren = i;
                break;
            }
        }

        for (size_t i = open_paren + 1; i < phrase->tokens.size(); ++i) {
            if (!phrase->tokens[i]) {
                continue;
            }

            if (phrase->tokens[i]->value == ")") {
                close_paren = i;
                break;
            }
        }

        const std::string function_name = joinDottedTokenRange(phrase->tokens, 0, open_paren);
        const auto arguments = parseCallArguments(phrase->tokens, open_paren, close_paren);
        parsed_phrase = std::make_shared<ParsedCallStatement>(function_name, arguments, phrase);
    } else {
        auto unknown = std::make_shared<ParsedUnknownPhrase>(joinTokenRange(phrase->tokens, 0, phrase->tokens.size()), phrase);
        parsed_phrase = unknown;
    }

    for (const std::shared_ptr<Phrase>& nested : phrase->nested_phrases) {
        parsed_phrase->addNested(parsePhrase(nested, parsed_phrase->kind));
    }

    return parsed_phrase;
}

ParsedPhrases parsePhrases(const Phrases& phrases) {
    ParsedPhrases parsed_phrases{};

    for (const std::shared_ptr<Phrase>& phrase : phrases) {
        parsed_phrases.push_back(parsePhrase(phrase));
    }

    return parsed_phrases;
}
