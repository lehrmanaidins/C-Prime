#pragma once

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "expression.cpp"

enum class SemanticTypeKind {
    Named,
    Tuple,
    Reference,
    Function
};

struct SemanticTypeRef {
    SemanticTypeKind kind = SemanticTypeKind::Named;
    std::string name;
    bool is_primitive = false;
    bool is_const = false;
    bool is_mutable = false;
    std::vector<SemanticTypeRef> tuple_elements;
    std::vector<std::string> array_dimensions;
    std::shared_ptr<SemanticTypeRef> reference_target;
    std::vector<SemanticTypeRef> function_parameters;
    std::shared_ptr<SemanticTypeRef> function_return_type;
};

static std::string normalizeTypeName(const std::string& raw_type) {
    std::string type = trim(raw_type);
    const std::string const_prefix = "const ";
    if (type.rfind(const_prefix, 0) == 0) {
        type = trim(type.substr(const_prefix.size()));
    }

    const std::string mutable_prefix = "mutable ";
    if (type.rfind(mutable_prefix, 0) == 0) {
        type = trim(type.substr(mutable_prefix.size()));
    }

    const std::string primitive_prefix = "primitive ";
    if (type.rfind(primitive_prefix, 0) == 0) {
        type = trim(type.substr(primitive_prefix.size()));
    }

    const size_t bracket_pos = type.find('[');
    if (bracket_pos != std::string::npos) {
        type = trim(type.substr(0, bracket_pos));
    }

    return type;
}

static bool consumeTypePrefix(std::string& type, const std::string& prefix) {
    if (type.rfind(prefix, 0) != 0) {
        return false;
    }

    type = trim(type.substr(prefix.size()));
    return true;
}

static std::string compactTypeSyntax(const std::string& raw_type) {
    std::string compact;
    for (char ch : raw_type) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        compact.push_back(ch);
    }
    return compact;
}

static SemanticTypeRef parseTypeRef(const std::string& raw_type) {
    std::string type = trim(raw_type);

    SemanticTypeRef base_type{};

    bool consumed_prefix = true;
    while (consumed_prefix) {
        consumed_prefix = false;
        if (consumeTypePrefix(type, "const ")) {
            base_type.is_const = true;
            consumed_prefix = true;
        } else if (consumeTypePrefix(type, "mutable ")) {
            base_type.is_mutable = true;
            consumed_prefix = true;
        } else if (consumeTypePrefix(type, "primitive ")) {
            base_type.is_primitive = true;
            consumed_prefix = true;
        }
    }

    std::vector<std::string> dimensions{};
    while (!type.empty() && type.back() == ']') {
        const size_t open = type.find_last_of('[');
        if (open == std::string::npos) {
            break;
        }

        dimensions.push_back(trim(type.substr(open + 1, type.size() - open - 2)));
        type = trim(type.substr(0, open));
    }

    std::reverse(dimensions.begin(), dimensions.end());

    const std::string compact = compactTypeSyntax(type);
    if (compact.rfind("reference<", 0) == 0 && compact.back() == '>') {
        const size_t open = type.find('<');
        const size_t close = type.rfind('>');
        base_type.kind = SemanticTypeKind::Reference;
        base_type.name = "reference";
        base_type.array_dimensions = dimensions;
        if (open != std::string::npos && close != std::string::npos && close > open) {
            base_type.reference_target = std::make_shared<SemanticTypeRef>(parseTypeRef(type.substr(open + 1, close - open - 1)));
        }
        return base_type;
    }

    if (type.rfind("function", 0) == 0) {
        base_type.kind = SemanticTypeKind::Function;
        base_type.name = "function";
        base_type.array_dimensions = dimensions;
        return base_type;
    }

    if (type.size() >= 2 && type.front() == '(' && type.back() == ')') {
        base_type.kind = SemanticTypeKind::Tuple;
        base_type.name = "tuple";
        base_type.array_dimensions = dimensions;

        const std::string inner = trim(type.substr(1, type.size() - 2));
        for (const auto& part : splitTopLevel(inner, ',')) {
            if (!part.empty()) {
                base_type.tuple_elements.push_back(parseTypeRef(part));
            }
        }

        return base_type;
    }

    base_type.kind = SemanticTypeKind::Named;
    base_type.name = type;
    base_type.array_dimensions = dimensions;
    return base_type;
}