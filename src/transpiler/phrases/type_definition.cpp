#pragma once

#include "../statement_data.cpp"

static void emitMemberFunction(
    std::string& output,
    CppEmitContext& context,
    const SemanticFunctionIR& function,
    const std::string& indent_str
) {
    context.pushLine(function.location, "function");
    const std::string return_type = emitTypeRef(function.return_type, context);
    std::string signature = nodiscardPrefix(return_type, function.is_discardable) + "auto " + function.name + "(";
    for (size_t indent = 0; indent < function.parameters.size(); ++indent) {
        if (indent > 0) signature += ", ";
        signature += emitTypeRef(function.parameters[indent].type, context) + " " + function.parameters[indent].name;
    }
    signature += ") -> " + return_type;
    appendLine(output, context, indent_str + signature + " {");
    if (function.has_requires) {
        context.required_headers.insert("<stdexcept>");
        appendLine(output, context, indent_str + "    if consteval (!(" + emitExpression(function.requires_clause, context) + ")) { throw std::runtime_error(\"function requires clause violated\"); }");
    }

    const std::string previous_return_value_name = context.current_return_value_name;
    const bool previous_function_has_ensures = context.current_function_has_ensures;
    const SemanticExpressionIR previous_function_ensures_clause = context.current_function_ensures_clause;
    context.current_return_value_name = function.return_value_name;
    context.current_function_has_ensures = function.has_ensures;
    context.current_function_ensures_clause = function.ensures_clause;

    const std::vector<SemanticImportIR> no_imports{};
    const CppEmitStatementData function_data{
        function.variable_declarations,
        function.assignments,
        function.type_definitions,
        function.struct_definitions,
        function.enum_definitions,
        no_imports,
        function.calls,
        function.returns,
        function.if_statements,
        function.while_statements,
        function.for_statements,
        function.else_statements,
        function.trivia
    };

    emitStatementList(output, context, function.body_order, function_data, indent_str.size() / 4 + 1);

    context.current_return_value_name = previous_return_value_name;
    context.current_function_has_ensures = previous_function_has_ensures;
    context.current_function_ensures_clause = previous_function_ensures_clause;

    appendLine(output, context, indent_str + "}");
}

enum class DomainUnderlyingKind {
    None,
    Signed_Integer,
    Unsigned_Integer,
    Floating,
    Boolean,
    Character,
    String,
    Domain,
    Composite,
    Struct
};

static DomainUnderlyingKind classifyDomainUnderlying(const SemanticTypeRef& base_type, const CppEmitContext& context) {
    if (!base_type.array_dimensions.empty() || base_type.kind == SemanticTypeKind::Tuple) {
        return DomainUnderlyingKind::Composite;
    }
    if (base_type.kind != SemanticTypeKind::Named) {
        return DomainUnderlyingKind::None;
    }

    const std::string& n = base_type.name;
    if (base_type.is_primitive) {
        if (n == "int8" || n == "int16" || n == "int32" || n == "int64") {
            return DomainUnderlyingKind::Signed_Integer;
        }
        if (n == "uint8" || n == "uint16" || n == "uint32" || n == "uint64") {
            return DomainUnderlyingKind::Unsigned_Integer;
        }
        if (n == "float32" || n == "float64") {
            return DomainUnderlyingKind::Floating;
        }
        if (n == "bool") {
            return DomainUnderlyingKind::Boolean;
        }
        if (n == "char8" || n == "char16" || n == "char32") {
            return DomainUnderlyingKind::Character;
        }
        return DomainUnderlyingKind::None;
    }

    if (n == "string") {
            return DomainUnderlyingKind::String;
    }

    if (n == "array") {
        return DomainUnderlyingKind::Composite;
    }

    if (context.struct_types.find(n) != context.struct_types.end()) {
        return DomainUnderlyingKind::Struct;
    }

    if (context.domain_base_types.find(n) != context.domain_base_types.end()) {
        return DomainUnderlyingKind::Domain;
    }

    return DomainUnderlyingKind::None;
}

// Follows a chain of domain-type-over-domain-type definitions down to the kind
// of the eventual root underlying type, so a domain type whose underlying is
// another domain type inherits that type's operator set.
static DomainUnderlyingKind resolvedDomainUnderlyingKind(const SemanticTypeRef& base_type, const CppEmitContext& context) {
    SemanticTypeRef current = base_type;
    for (int guard = 0; guard < 64; ++guard) {
        const DomainUnderlyingKind kind = classifyDomainUnderlying(current, context);
        if (kind != DomainUnderlyingKind::Domain) {
            return kind;
        }
        const auto it = context.domain_base_types.find(current.name);
        if (it == context.domain_base_types.end()) {
            return DomainUnderlyingKind::None;
        }
        current = it->second;
    }
    return DomainUnderlyingKind::None;
}

// Walks a chain of domain-type-over-domain-type definitions down to the first
// underlying type that is not itself a domain type, so the wrapper struct is
// built directly over the eventual root (e.g. a primitive) rather than over an
// intermediate domain type that does not implicitly decay.
static SemanticTypeRef resolvedDomainUnderlyingType(const SemanticTypeRef& base_type, const CppEmitContext& context) {
    SemanticTypeRef current = base_type;
    for (int guard = 0; guard < 64; ++guard) {
        if (classifyDomainUnderlying(current, context) != DomainUnderlyingKind::Domain) {
            return current;
        }
        const auto it = context.domain_base_types.find(current.name);
        if (it == context.domain_base_types.end()) {
            return current;
        }
        current = it->second;
    }
    return current;
}

// Builds a member function that simply forwards to the wrapped `value`, so a
// domain type exposes the member functions declared on its underlying type(s).
static std::string emitForwardingMemberFunction(CppEmitContext& context, const SemanticFunctionIR& function) {
    const std::string return_type = emitTypeRef(function.return_type, context);
    std::string signature = nodiscardPrefix(return_type, function.is_discardable) + "auto " + function.name + "(";
    std::string call_arguments;
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        if (i > 0) {
            signature += ", ";
            call_arguments += ", ";
        }
        const std::string parameter_name = function.parameters[i].name;
        signature += emitTypeRef(function.parameters[i].type, context) + " " + parameter_name;
        call_arguments += parameter_name;
    }
    signature += ") -> " + return_type;
    return signature + " { return value." + function.name + "(" + call_arguments + "); } ";
}

// Collects forwarding member functions for every domain type in the underlying
// chain of `base_type`. A C-Prime `type` definition may declare member
// functions, and a domain type built on top of another domain type should
// expose them too.
static std::string collectUnderlyingMemberFunctions(CppEmitContext& context, const SemanticTypeRef& base_type) {
    std::string forwarded;
    std::unordered_set<std::string> seen_names;
    SemanticTypeRef current = base_type;

    for (int guard = 0; guard < 64; ++guard) {
        if (current.kind != SemanticTypeKind::Named) {
            break;
        }

        const auto functions_it = context.domain_member_functions.find(current.name);
        if (functions_it != context.domain_member_functions.end()) {
            for (const auto& function : functions_it->second) {
                if (seen_names.insert(function.name).second) {
                    forwarded += emitForwardingMemberFunction(context, function);
                }
            }
        }

        const auto base_it = context.domain_base_types.find(current.name);
        if (base_it == context.domain_base_types.end()) {
            break;
        }
        current = base_it->second;
    }

    return forwarded;
}

// Emits the operator / accessor set for a domain type's wrapper struct, chosen
// at transpile time from the underlying type:
//   * integral primitive -> arithmetic, bitwise, comparison, compound
//   * floating primitive  -> arithmetic (no %/bitwise), comparison, compound
//   * bool primitive      -> comparison only
//   * composite (array / tuple) -> comparison, and `[]` for arrays (element
//     re-wrapped in the domain type)
//   * struct              -> a forwarding accessor per field of the struct
// Value-producing operators re-wrap their result in the domain type so it never
// implicitly decays to its underlying representation.
// Emits the struct-body operator / accessor set for a domain type's wrapper
// struct, chosen at transpile time from the (possibly chained) underlying type.
// The trailing argument to the CPRIME_*_MEMBERS macros carries the forwarding
// member functions collected from every underlying `type` in the chain.
static void emitDomainOperators(
    std::string& output,
    CppEmitContext& context,
    const std::string& name,
    const SemanticTypeRef& base_type,
    std::string base_type_str,
    const std::string& indent_str
) {
    const DomainUnderlyingKind kind = resolvedDomainUnderlyingKind(base_type, context);
    const std::string indent = indent_str + "    ";
    const std::string member_functions = collectUnderlyingMemberFunctions(context, base_type);

    // When the underlying type is itself a domain type, build the wrapper over
    // the deepest non-domain underlying instead of the intermediate domain type.
    const SemanticTypeRef root_underlying = resolvedDomainUnderlyingType(base_type, context);
    if (root_underlying.name != base_type.name || root_underlying.kind != base_type.kind) {
        base_type_str = emitTypeRef(root_underlying, context);
    }

    if (kind == DomainUnderlyingKind::Struct) {
        const auto fields_it = context.struct_fields.find(base_type.name);
        if (fields_it != context.struct_fields.end()) {
            for (const auto& field : fields_it->second) {
                appendLine(output, context, "");
                appendLine(output, context, indent + "[[nodiscard]] auto& " + field + "() { return value." + field + "; }");
                appendLine(output, context, indent + "[[nodiscard]] const auto& " + field + "() const { return value." + field + "; }");
            }
        }
        if (!member_functions.empty()) {
            appendLine(output, context, "");
            appendLine(output, context, indent + member_functions);
        }
        return;
    }

    std::string members_macro;
    switch (kind) {
        case DomainUnderlyingKind::Signed_Integer:   members_macro = "CPRIME_SIGNED_INTEGER_TYPE"; break;
        case DomainUnderlyingKind::Unsigned_Integer: members_macro = "CPRIME_UNSIGNED_INTEGER_TYPE"; break;
        case DomainUnderlyingKind::Floating:         members_macro = "CPRIME_FLOAT_TYPE"; break;
        case DomainUnderlyingKind::Boolean:          members_macro = "CPRIME_BOOLEAN_TYPE"; break;
        case DomainUnderlyingKind::Character:        members_macro = "CPRIME_CHARACTER_TYPE"; break;
        case DomainUnderlyingKind::Composite:        members_macro = "CPRIME_COMPOSITE_TYPE"; break;
        default:
            throw std::runtime_error("Transpile error: cannot resolve the underlying type of domain type '" + name + "'");
    }

    // A composite underlying carries a top-level comma, so the CPRIME_*_TYPE
    // macros expect it parenthesised; other underlyings are passed bare.
    if (kind == DomainUnderlyingKind::Composite) {
        base_type_str = "(" + base_type_str + ")";
    } else if (base_type_str.find(' ') != std::string::npos) {
        base_type_str = "(" + base_type_str + ")";
    }

    if (!member_functions.empty()) {
        appendLine(output, context, indent + members_macro + "(" + name + ", " + base_type_str + ", " + member_functions + ")");
    } else {
        appendLine(output, context, indent + members_macro + "(" + name + ", " + base_type_str + ")");
    }
}

static void emitTypeDefinitionStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticTypeDefinitionIR& type_def,
    const std::string& indent_str
) {
    const bool is_generic = !type_def.template_parameters.empty();

    bool has_template_array_dim = false;
    for (const auto& dim : type_def.base_type.array_dimensions) {
        if (dim.empty()) {
            has_template_array_dim = true;
            break;
        }
    }

    if (is_generic && has_template_array_dim) {
        throw std::runtime_error("Transpile error: generic type '" + type_def.name
            + "' needs a fixed array length (unsized '[]' is not supported for generic types)");
    }

    const bool is_simple_type_alias = !type_def.has_requires && !type_def.has_ensures && type_def.member_functions.empty();
    if (!is_generic && is_simple_type_alias && has_template_array_dim) {
        context.type_aliases[type_def.name] = type_def.base_type;
        context.pushLine(type_def.location, "type");
        return;
    }

    context.domain_struct_types.insert(type_def.name);
    context.domain_base_types[type_def.name] = type_def.base_type;
    context.domain_member_functions[type_def.name] = type_def.member_functions;
    context.pushLine(type_def.location, "type");
    context.required_headers.insert("<stdexcept>");

    const std::string base_type_str = emitTypeRef(type_def.base_type, context);
    const std::string name = type_def.name;

    emitDomainOperators(output, context, name, type_def.base_type, base_type_str, indent_str + "   ");
}
