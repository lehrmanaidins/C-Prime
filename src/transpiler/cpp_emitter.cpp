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
    if (constraint.find("int") != std::string::npos) {
        constraint += " || std::same_as<" + parameter.name + ", int>";
    }
    if (constraint.find("float") != std::string::npos) {
        constraint += " || std::same_as<" + parameter.name + ", float> || std::same_as<" + parameter.name + ", double>";
    }
    if (constraint.find("char") != std::string::npos) {
        constraint += " || std::same_as<" + parameter.name + ", char>";
    }
    if (constraint.find("bool") != std::string::npos) {
        constraint += " || std::same_as<" + parameter.name + ", bool>";
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
        case SemanticStatementKind::Trivia: {
            const SemanticTriviaIR& trivia = data.trivia[ref.index];
            if (!trivia.suffix.empty()) {
                if (!output.empty() && output.back() == '\n') {
                    output.pop_back();
                    output += " ";
                    output += trivia.suffix;
                    output += "\n";
                }
                break;
            }
            for (const auto& line : trivia.lines) {
                appendLine(output, context, line.empty() ? "" : i + line);
            }
            break;
        }
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
        case SemanticStatementKind::Function:
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

static void emitTopLevelFunction(
    std::string& body,
    CppEmitContext& context,
    const SemanticProgram& program,
    const SemanticFunctionIR& function
) {
    context.pushLine(function.location, "function");

    for (const auto& line : function.leading_trivia_lines) {
        appendLine(body, context, line);
    }

    const bool is_main = function.name == "main";
    if (!function.template_parameters.empty()) {
        std::string template_line = "template <typename " + function.template_parameters.front().name;
        for (size_t i = 1; i < function.template_parameters.size(); ++i) {
            template_line += ", typename " + function.template_parameters[i].name;
        }
        template_line += ">";
        appendLine(body, context, template_line);

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
    std::string signature = nodiscardPrefix(return_type, function.is_discardable) + "auto " + function.name + "(";
    for (size_t i = 0; i < function.parameters.size(); ++i) {
        if (i > 0) signature += ", ";
        signature += emitTypeRef(function.parameters[i].type, context) + " " + function.parameters[i].name;
    }
    signature += ") -> " + return_type;

    appendLine(body, context, signature + " {");
    if (function.has_requires) {
        context.required_headers.insert("<stdexcept>");
        appendLine(body, context, "    if constexpr (!(" + emitExpression(function.requires_clause, context) + ")) { throw std::runtime_error(\"function requires clause violated\"); }");
    }

    const std::string previous_return_value_name = context.current_return_value_name;
    const bool previous_function_has_ensures = context.current_function_has_ensures;
    const SemanticExpressionIR previous_function_ensures_clause = context.current_function_ensures_clause;
    context.current_return_value_name = function.return_value_name;
    context.current_function_has_ensures = function.has_ensures;
    context.current_function_ensures_clause = function.ensures_clause;

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
        function.else_statements,
        function.trivia
    };

    emitStatementList(body, context, function.body_order, function_data, 1);

    context.current_return_value_name = previous_return_value_name;
    context.current_function_has_ensures = previous_function_has_ensures;
    context.current_function_ensures_clause = previous_function_ensures_clause;

    for (const auto& line : function.trailing_trivia_lines) {
        appendLine(body, context, line.empty() ? "" : "    " + line);
    }

    if (is_main) {
        appendLine(body, context, "    return EXIT_SUCCESS;");
    }
    appendLine(body, context, "}");
}

CppEmitResult emitCpp(const SemanticProgram& program) {
    CppEmitResult result{};
    CppEmitContext context{};
    context.union_members = program.union_members;
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
        program.else_statements,
        program.trivia
    };

    for (const auto& statement : program.top_level_order) {
        if (statement.kind == SemanticStatementKind::Function) {
            emitTopLevelFunction(body, context, program, program.functions[statement.index]);
        } else {
            emitStatementRef(body, context, statement, top_level_data, 0);
        }
    }
    for (const auto& line : program.trailing_trivia_lines) {
        appendLine(body, context, line);
    }

    std::string output;
    output += "// Generated by C-Prime transpiler\n";

    context.required_headers.insert("<cstdlib>");
    context.required_headers.insert("\"io.hpp\"");
    context.required_headers.insert("\"c-prime.hpp\"");

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

