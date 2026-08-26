
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "preparser.cpp"
#include "parsed_phrase.cpp"
#include "phrases/assignment.cpp"
#include "phrases/enum_definition.cpp"
#include "phrases/enum_value_definition.cpp"
#include "phrases/function.cpp"
#include "phrases/parameter_declaration.cpp"
#include "phrases/parameter_definition.cpp"
#include "phrases/statement.cpp"
#include "phrases/struct_definition.cpp"
#include "phrases/type_definition.cpp"
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
        || value == "primitive"
        || value == "type"
        || value == "struct"
        || value == "enum"
        || value == "function";
}

static std::string findMissingSemicolonBeforeToken(const Tokens& tokens) {
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;

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
        }

        if (i == 0 || paren_depth > 0 || bracket_depth > 0 || brace_depth > 0) {
            continue;
        }

        if (!isDeclarationStarter(value)) {
            continue;
        }

        if (!tokens[i - 1]) {
            continue;
        }

        const std::string prev = tokens[i - 1]->value;
        if (isDeclarationStarter(prev) || prev == ":" || prev == ",") {
            continue;
        }

        return value;
    }

    return "";
}

static void validatePhrase(const std::shared_ptr<Phrase>& phrase, ParsedPhraseKind parent_kind) {
    if (!phrase) {
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

static bool isFunctionPhrase(const std::shared_ptr<Phrase>& phrase, size_t& open_paren, size_t& close_paren) {
    if (!phrase || phrase->tokens.empty()) {
        return false;
    }

    if (phrase->tokens[0] && phrase->tokens[0]->value == "function") {
        return findParentheses(phrase->tokens, open_paren, close_paren);
    }

    return false;
}

static bool isTypeDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() >= 4 && phrase->tokens[0] && phrase->tokens[0]->value == "type" && hasTokenValue(phrase->tokens, ":");
}

static bool isStructDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() >= 2 && phrase->tokens[0] && phrase->tokens[0]->value == "struct";
}

static bool isEnumDefinitionPhrase(const std::shared_ptr<Phrase>& phrase) {
    return phrase && phrase->tokens.size() >= 2 && phrase->tokens[0] && phrase->tokens[0]->value == "enum";
}

static bool isVariableDeclarationPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 2) {
        return false;
    }

    if (phrase->tokens[0] && (phrase->tokens[0]->value == "type" || phrase->tokens[0]->value == "function")) {
        return false;
    }

    if (phrase->tokens[0] && (phrase->tokens[0]->value == "struct" || phrase->tokens[0]->value == "enum")) {
        return false;
    }

    if (hasTokenValue(phrase->tokens, ":")) {
        return false;
    }

    const size_t equal_index = findTokenIndex(phrase->tokens, "=");
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

static bool isAssignmentPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 3) {
        return false;
    }

    const size_t equal_index = findTokenIndex(phrase->tokens, "=");
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

static bool isBuiltinCallPhrase(const std::shared_ptr<Phrase>& phrase) {
    if (!phrase || phrase->tokens.size() < 3) {
        return false;
    }

    if (!phrase->tokens[0]) {
        return false;
    }

    const std::string name = phrase->tokens[0]->value;
    if (name != "print" && name != "println") {
        return false;
    }

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

    if (open_paren == phrase->tokens.size()) {
        return false;
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

    return close_paren != phrase->tokens.size() && close_paren == phrase->tokens.size() - 1;
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
        const std::string return_type = joinTokenRange(phrase->tokens, 1, open_paren - 1);
        auto function = std::make_shared<ParsedFunction>(function_name, return_type, phrase);
        parseFunctionParameters(phrase->tokens, open_paren, close_paren, phrase, function);
        parsed_phrase = function;
    } else if (isTypeDefinitionPhrase(phrase)) {
        const std::string type_name = phrase->tokens[1] ? phrase->tokens[1]->value : "";
        const size_t colon_index = findTokenIndex(phrase->tokens, ":");
        const std::string base_type = joinTokenRange(phrase->tokens, colon_index + 1, phrase->tokens.size());
        parsed_phrase = std::make_shared<ParsedTypeDefinition>(type_name, base_type, phrase);
    } else if (isStructDefinitionPhrase(phrase)) {
        const std::string struct_name = phrase->tokens[1] ? phrase->tokens[1]->value : "";
        parsed_phrase = std::make_shared<ParsedStructDefinition>(struct_name, phrase);
    } else if (isEnumDefinitionPhrase(phrase)) {
        const std::string enum_name = phrase->tokens[1] ? phrase->tokens[1]->value : "";
        parsed_phrase = std::make_shared<ParsedEnumDefinition>(enum_name, phrase);
    } else if (isVariableDeclarationPhrase(phrase)) {
        const size_t equal_index = findTokenIndex(phrase->tokens, "=");
        const bool is_mutable = phrase->tokens[0] && phrase->tokens[0]->value == "mutable";
        const bool has_initializer = equal_index != phrase->tokens.size();
        const size_t name_index = has_initializer
            ? (equal_index > 0 ? equal_index - 1 : 0)
            : (phrase->tokens.size() - 1);
        const std::string name = phrase->tokens[name_index] ? phrase->tokens[name_index]->value : "";
        const size_t type_begin = is_mutable ? 1 : 0;
        const std::string type_name = joinTokenRange(phrase->tokens, type_begin, name_index);
        const std::string initializer = has_initializer
            ? joinTokenRange(phrase->tokens, equal_index + 1, phrase->tokens.size())
            : "";
        parsed_phrase = std::make_shared<ParsedVariableDeclaration>(type_name, name, initializer, is_mutable, phrase);
    } else if (isAssignmentPhrase(phrase)) {
        const size_t equal_index = findTokenIndex(phrase->tokens, "=");
        const std::string left = joinTokenRange(phrase->tokens, 0, equal_index);
        const std::string right = joinTokenRange(phrase->tokens, equal_index + 1, phrase->tokens.size());
        parsed_phrase = std::make_shared<ParsedAssignment>(left, right, phrase);
    } else if (isBuiltinCallPhrase(phrase)) {
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

        const std::string function_name = phrase->tokens[0] ? phrase->tokens[0]->value : "";
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
