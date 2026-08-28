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
    std::string signature = return_type + " " + sanitizeIdentifier(function.name) + "(";
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        if (i > 0) signature += ", ";
        signature += emitTypeRef(function.parameters[i].type, context) + " " + sanitizeIdentifier(function.parameters[i].name);
    }
    signature += ")";
    appendLine(output, context, indent_str + signature + " {");

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
        function.else_statements
    };

    emitStatementList(output, context, function.body_order, function_data, indent_str.size() / 4 + 1);

    appendLine(output, context, indent_str + "}");
}

static void emitTypeDefinitionStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticTypeDefinitionIR& type_def,
    const std::string& indent_str
) {
    bool has_template_array_dim = false;
    for (const auto& dim : type_def.base_type.array_dimensions) {
        if (dim.empty()) {
            has_template_array_dim = true;
            break;
        }
    }

    if (!type_def.has_requires && !type_def.has_ensures && type_def.member_functions.empty() && has_template_array_dim) {
        context.type_aliases[type_def.name] = type_def.base_type;
        context.pushLine(type_def.location, "type");
        appendLine(output, context, indent_str + "using " + sanitizeIdentifier(type_def.name) + " = " + emitTypeRef(type_def.base_type, context) + ";");
        return;
    }

    context.domain_struct_types.insert(type_def.name);
    context.pushLine(type_def.location, "type");
    context.required_headers.insert("<stdexcept>");
    context.required_headers.insert("<utility>");

    const std::string base_type_str = emitTypeRef(type_def.base_type, context);
    const std::string name = sanitizeIdentifier(type_def.name);

    appendLine(output, context, indent_str + "struct " + name + " {");
    appendLine(output, context, indent_str + "    " + base_type_str + " value;");
    appendLine(output, context, "");
    appendLine(output, context, indent_str + "    template <typename T>");
    appendLine(output, context, indent_str + "    " + name + "(T&& value_arg) : value(std::forward<T>(value_arg)) {");
    if (type_def.has_requires) {
        appendLine(output, context, indent_str + "        if (!(" + emitExpression(type_def.requires_clause, context) + ")) { throw std::runtime_error(\"" + type_def.name + ": requires clause violated\"); }");
    }
    if (type_def.has_ensures) {
        appendLine(output, context, indent_str + "        if (!(" + emitExpression(type_def.ensures_clause, context) + ")) { throw std::runtime_error(\"" + type_def.name + ": ensures clause violated\"); }");
    }
    appendLine(output, context, indent_str + "    }");
    appendLine(output, context, "");
    appendLine(output, context, indent_str + "    template <typename T>");
    appendLine(output, context, indent_str + "    " + name + "& operator=(T&& value_arg) { *this = " + name + "(std::forward<T>(value_arg)); return *this; }");
    appendLine(output, context, "");
    appendLine(output, context, indent_str + "    operator " + base_type_str + "() const { return value; }");

    for (const auto& member_function : type_def.member_functions) {
        appendLine(output, context, "");
        emitMemberFunction(output, context, member_function, indent_str + "    ");
    }

    appendLine(output, context, indent_str + "};");
}
