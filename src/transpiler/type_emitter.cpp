#pragma once

#include "cpp_emit_context.cpp"

static SemanticTypeRef resolveAliasTypeRef(const SemanticTypeRef& type, CppEmitContext& context) {
    if (type.kind != SemanticTypeKind::Named) {
        return type;
    }

    auto alias_it = context.type_aliases.find(type.name);
    if (alias_it == context.type_aliases.end()) {
        return type;
    }

    SemanticTypeRef resolved = alias_it->second;

    size_t usage_dim_index = 0;
    for (auto& dim : resolved.array_dimensions) {
        if (dim.empty() && usage_dim_index < type.array_dimensions.size()) {
            dim = type.array_dimensions[usage_dim_index++];
        }
    }

    while (usage_dim_index < type.array_dimensions.size()) {
        resolved.array_dimensions.push_back(type.array_dimensions[usage_dim_index++]);
    }

    return resolved;
}

static std::string emitTypeRef(const SemanticTypeRef& type, CppEmitContext& context) {
    const SemanticTypeRef resolved_type = resolveAliasTypeRef(type, context);
    std::string base;

    if (resolved_type.kind == SemanticTypeKind::Tuple) {
        context.required_headers.insert("<tuple>");
        base = "std::tuple<";
        for (size_t i = 0; i < resolved_type.tuple_elements.size(); ++i) {
            if (i > 0) base += ", ";
            base += emitTypeRef(resolved_type.tuple_elements[i], context);
        }
        base += ">";
    } else if (resolved_type.kind == SemanticTypeKind::Reference && resolved_type.reference_target) {
        base = emitTypeRef(*resolved_type.reference_target, context) + "&";
    } else if (resolved_type.kind == SemanticTypeKind::Function) {
        base = "auto";
    } else {
        base = mapPrimitiveType(resolved_type.name, context);
    }

    for (auto it = resolved_type.array_dimensions.rbegin(); it != resolved_type.array_dimensions.rend(); ++it) {
        if (it->empty()) {
            context.required_headers.insert("<vector>");
            base = "std::vector<" + base + ">";
            continue;
        }

        context.required_headers.insert("<array>");
        base = "std::array<" + base + ", " + *it + ">";
    }

    return base;
}
