#pragma once

#include "cpp_emit_context.cpp"

static SemanticTypeRef resolveAliasTypeRef(const SemanticTypeRef& type, CppEmitContext& context) {
    if (type.kind != SemanticTypeKind::Named || !type.generic_arguments.empty()) {
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

static std::string emitTypeRef(const SemanticTypeRef& type, CppEmitContext& context);

static void collectUnionMemberTypes(
    const std::string& type_name,
    CppEmitContext& context,
    std::vector<SemanticTypeRef>& members
) {
    const auto union_it = context.union_members.find(normalizeTypeName(type_name));
    if (union_it == context.union_members.end()) {
        members.push_back(parseTypeRef(type_name));
        return;
    }

    for (const auto& member : union_it->second) {
        collectUnionMemberTypes(member, context, members);
    }
}

static std::string emitUnionTypeRef(const std::string& type_name, CppEmitContext& context) {
    context.required_headers.insert("<variant>");

    std::vector<SemanticTypeRef> members{};
    collectUnionMemberTypes(type_name, context, members);

    std::string base = "std::variant<";
    for (size_t i = 0; i < members.size(); ++i) {
        if (i > 0) base += ", ";
        base += emitTypeRef(members[i], context);
    }
    base += ">";
    return base;
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
        context.required_headers.insert("\"memory.hpp\"");
        const std::string qualifier = resolved_type.reference_target->is_mutable ? "" : "const ";
        base = "cprime::reference<" + qualifier + emitTypeRef(*resolved_type.reference_target, context) + ">";
    } else if (resolved_type.kind == SemanticTypeKind::Pointer && resolved_type.pointer_target) {
        context.required_headers.insert("\"memory.hpp\"");
        const std::string qualifier = resolved_type.pointer_target->is_mutable ? "" : "const ";
        base = "cprime::pointer<" + qualifier + emitTypeRef(*resolved_type.pointer_target, context) + ">";
    } else if (resolved_type.kind == SemanticTypeKind::Function) {
        base = "auto";
    } else if (!resolved_type.generic_arguments.empty()) {
        std::string qualified_name = resolved_type.name;
        std::vector<SemanticTypeRef> generic_arguments = resolved_type.generic_arguments;
        if (resolved_type.name == "array" || resolved_type.name == "string") {
            context.required_headers.insert("\"c-prime.hpp\"");
            qualified_name = "cprime::" + resolved_type.name;
        }
        // C-Prime source writes `string<Size>` or `string<T, Size>` (element type
        // first, matching how it reads), but the runtime declares
        // `cprime::string<Size, T = char8>` so a bare `string<Size>` can default
        // the element type. Reorder the two-argument form to match.
        if (resolved_type.name == "string" && generic_arguments.size() == 2) {
            std::swap(generic_arguments[0], generic_arguments[1]);
        }
        base = qualified_name + "<";
        for (size_t i = 0; i < generic_arguments.size(); ++i) {
            if (i > 0) base += ", ";
            base += emitTypeRef(generic_arguments[i], context);
        }
        base += ">";
    } else if (context.union_members.find(normalizeTypeName(resolved_type.name)) != context.union_members.end()) {
        base = emitUnionTypeRef(resolved_type.name, context);
    } else {
        base = mapPrimitiveType(resolved_type.name, context);
    }

    // A domain type defined from an unsized `[]` element type is emitted as a
    // struct templated on the element count, so a use-site length instantiates it
    // as `Name<N>` rather than wrapping it in another array.
    if (context.domain_array_template_types.count(resolved_type.name) != 0
        && resolved_type.array_dimensions.size() == 1
        && !resolved_type.array_dimensions.front().empty()) {
        context.required_headers.insert("\"c-prime.hpp\"");
        return base + "<" + resolved_type.array_dimensions.front() + ">";
    }

    for (auto it = resolved_type.array_dimensions.rbegin(); it != resolved_type.array_dimensions.rend(); ++it) {
        if (it->empty()) {
            throw std::runtime_error("Transpile error: array type '" + resolved_type.name + "[]' requires a length at the use site");
        }

        context.required_headers.insert("\"c-prime.hpp\"");
        base = "cprime::array<" + base + ", " + *it + ">";
    }

    return base;
}
