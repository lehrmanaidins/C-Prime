#pragma once

#include "statement_data.cpp"
#include "phrases/assignment.cpp"
#include "phrases/call.cpp"
#include "phrases/control_flow.cpp"
#include "phrases/enum_definition.cpp"
#include "phrases/import.cpp"
#include "phrases/return_statement.cpp"
#include "phrases/struct_definition.cpp"
#include "phrases/type_definition.cpp"
#include "phrases/variable_declaration.cpp"

static std::vector<SemanticTypeRef> primitiveCategoryTypes(const std::string& category) {
    const std::vector<std::string> names = category == "Integral"
        ? std::vector<std::string>{"int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64"}
        : std::vector<std::string>{"float32", "float64"};

    std::vector<SemanticTypeRef> types;
    for (const auto& name : names) {
        types.push_back(parseTypeRef(name));
    }
    return types;
}

static std::string emitTemplateConstraint(const SemanticTemplateParameterIR& parameter, CppEmitContext& context) {
    const std::vector<SemanticTypeRef> allowed_types = parameter.category.empty()
        ? parameter.allowed_types
        : primitiveCategoryTypes(parameter.category);
    if (allowed_types.empty()) {
        return "";
    }

    context.required_headers.insert("<concepts>");
    std::string constraint = "(";
    for (size_t i = 0; i < allowed_types.size(); ++i) {
        if (i > 0) constraint += " || ";
        constraint += "std::same_as<" + parameter.name + ", " + emitTypeRef(allowed_types[i], context) + ">";
    }
    return constraint + ")";
}

static void emitStatementRef(
    std::string& output,
    CppEmitContext& context,
    const SemanticStatementRef& ref,
    const CppEmitStatementData& data,
    size_t indent_depth
) {
    const std::string i = indent(indent_depth);

    switch (ref.kind) {
        case SemanticStatementKind::VariableDeclaration:
            emitVariableDeclarationStatement(output, context, data.vars[ref.index], i);
            break;
        case SemanticStatementKind::Assignment:
            emitAssignmentStatement(output, context, data.assigns[ref.index], i);
            break;
        case SemanticStatementKind::TypeDefinition:
            emitTypeDefinitionStatement(output, context, data.types[ref.index], i);
            break;
        case SemanticStatementKind::StructDefinition:
            emitStructDefinitionStatement(output, context, data.structs[ref.index], i);
            break;
        case SemanticStatementKind::EnumDefinition:
            emitEnumDefinitionStatement(output, context, data.enums[ref.index], i);
            break;
        case SemanticStatementKind::Import:
            emitImportStatement(output, context, data.imports[ref.index]);
            break;
        case SemanticStatementKind::Call:
            emitCallStatement(output, context, data.calls[ref.index], i);
            break;
        case SemanticStatementKind::Return:
            emitReturnStatement(output, context, data.returns[ref.index], i);
            break;
        case SemanticStatementKind::Break:
            emitBreakStatement(output, context, i);
            break;
        case SemanticStatementKind::Continue:
            emitContinueStatement(output, context, i);
            break;
        case SemanticStatementKind::If:
            emitIfStatement(output, context, data.ifs[ref.index], data, i, indent_depth);
            break;
        case SemanticStatementKind::Else:
            emitElseStatement(output, context, data.elses[ref.index], data, i, indent_depth);
            break;
        case SemanticStatementKind::While:
            emitWhileStatement(output, context, data.whiles[ref.index], data, i, indent_depth);
            break;
        case SemanticStatementKind::For:
            emitForStatement(output, context, data.fors[ref.index], data, i, indent_depth);
            break;
    }
}

static void emitStatementList(
    std::string& output,
    CppEmitContext& context,
    const std::vector<SemanticStatementRef>& order,
    const CppEmitStatementData& data,
    size_t indent_depth
) {
    for (const auto& ref : order) {
        emitStatementRef(output, context, ref, data, indent_depth);
    }
}

CppEmitResult emitCpp(const SemanticProgram& program) {
    CppEmitResult result{};
    CppEmitContext context{};
    const std::vector<SemanticReturnIR> no_top_level_returns{};

    std::string body;

    const CppEmitStatementData top_level_data{
        program.global_variables,
        program.global_assignments,
        program.type_definitions,
        program.struct_definitions,
        program.enum_definitions,
        program.imports,
        program.calls,
        no_top_level_returns,
        program.if_statements,
        program.while_statements,
        program.for_statements,
        program.else_statements
    };

    for (const auto& statement : program.top_level_order) {
        emitStatementRef(body, context, statement, top_level_data, 0);
    }

    for (const auto& function : program.functions) {
        context.pushLine(function.location, "function");

        const bool is_main = function.name == "main";
        if (!function.template_parameters.empty()) {
            appendLine(body, context, "template <typename " + function.template_parameters.front().name);
            for (size_t i = 1; i < function.template_parameters.size(); ++i) {
                body += ", typename " + function.template_parameters[i].name;
            }
            body += ">\n";
            ++context.current_cpp_line;

            std::string requires_clause;
            for (const auto& parameter : function.template_parameters) {
                const std::string constraint = emitTemplateConstraint(parameter, context);
                if (constraint.empty()) {
                    continue;
                }
                if (!requires_clause.empty()) {
                    requires_clause += " && ";
                }
                requires_clause += constraint;
            }
            if (!requires_clause.empty()) {
                appendLine(body, context, "requires " + requires_clause);
            }
        }
        const std::string return_type = is_main ? "int" : emitTypeRef(function.return_type, context);
        std::string signature = return_type + " " + sanitizeIdentifier(function.name) + "(";
        for (size_t i = 0; i < function.parameters.size(); ++i) {
            if (i > 0) signature += ", ";
            signature += emitTypeRef(function.parameters[i].type, context) + " " + sanitizeIdentifier(function.parameters[i].name);
        }
        signature += ")";

        appendLine(body, context, signature + " {");

        const CppEmitStatementData function_data{
            function.variable_declarations,
            function.assignments,
            function.type_definitions,
            function.struct_definitions,
            function.enum_definitions,
            program.imports,
            function.calls,
            function.returns,
            function.if_statements,
            function.while_statements,
            function.for_statements,
            function.else_statements
        };

        emitStatementList(body, context, function.body_order, function_data, 1);

        if (is_main) {
            appendLine(body, context, "    return 0;");
        }
        appendLine(body, context, "}");
    }

    std::string output;
    output += "// Generated by C-Prime transpiler\n";

    context.required_headers.insert("\"src/runtime/c-prime.hpp\"");

    if (context.required_headers.empty()) {
        context.required_headers.insert("<cstdint>");
    }

    for (const auto& header : context.required_headers) {
        output += "#include " + header + "\n";
    }

    output += "\n";
    output += body;

    result.code = output;
    result.source_map = context.source_map;
    return result;
}

