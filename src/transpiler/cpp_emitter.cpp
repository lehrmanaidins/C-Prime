#pragma once

#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../semantic/semantic.cpp"

struct CppSourceMapEntry {
    size_t cpp_line;
    size_t cprime_line;
    size_t cprime_column;
    std::string note;
};

struct CppEmitResult {
    std::string code;
    std::vector<CppSourceMapEntry> source_map;
    std::vector<std::string> diagnostics;
};

struct CppEmitContext {
    std::unordered_set<std::string> required_headers;
    std::unordered_map<std::string, SemanticTypeRef> type_aliases;
    std::unordered_set<std::string> struct_types;
    std::vector<CppSourceMapEntry> source_map;
    size_t current_cpp_line = 1;

    void pushLine(const SourceLocation& loc, const std::string& note) {
        source_map.push_back({current_cpp_line, loc.line, loc.column, note});
    }
};

static std::string sanitizeIdentifier(const std::string& name) {
    static const std::set<std::string> cpp_keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
        "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept",
        "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return",
        "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
        "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int",
        "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
        "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "requires", "return",
        "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "template",
        "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned",
        "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };

    if (cpp_keywords.find(name) != cpp_keywords.end()) {
        return name + "_cp";
    }

    return name;
}

static std::string mapPrimitiveType(const std::string& type_name, CppEmitContext& context) {
    context.required_headers.insert("<cstdint>");

    if (type_name == "bool") return "bool";
    if (type_name == "char8") return "char";
    if (type_name == "char16") return "char16_t";
    if (type_name == "char32") return "char32_t";
    if (type_name == "int8") return "int8_t";
    if (type_name == "int16") return "int16_t";
    if (type_name == "int32") return "int32_t";
    if (type_name == "int64") return "int64_t";
    if (type_name == "uint8") return "uint8_t";
    if (type_name == "uint16") return "uint16_t";
    if (type_name == "uint32") return "uint32_t";
    if (type_name == "uint64") return "uint64_t";
    if (type_name == "float32") return "float";
    if (type_name == "float64") return "double";
    if (type_name == "void") return "void";

    return sanitizeIdentifier(type_name);
}

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

static std::string emitExpression(const SemanticExpressionIR& expression, CppEmitContext& context) {
    switch (expression.kind) {
        case SemanticExpressionKind::Identifier:
            return sanitizeIdentifier(expression.text);
        case SemanticExpressionKind::Literal:
            return expression.text;
        case SemanticExpressionKind::Binary:
            if (expression.children.size() == 2) {
                return "(" + emitExpression(expression.children[0], context) + " " + expression.operator_symbol + " " + emitExpression(expression.children[1], context) + ")";
            }
            return expression.text;
        case SemanticExpressionKind::Call: {
            std::string call = sanitizeIdentifier(expression.operator_symbol) + "(";
            for (size_t i = 0; i < expression.children.size(); ++i) {
                if (i > 0) call += ", ";
                call += emitExpression(expression.children[i], context);
            }
            call += ")";
            return call;
        }
        case SemanticExpressionKind::TupleLiteral: {
            std::string tuple = "std::make_tuple(";
            context.required_headers.insert("<tuple>");
            for (size_t i = 0; i < expression.children.size(); ++i) {
                if (i > 0) tuple += ", ";
                tuple += emitExpression(expression.children[i], context);
            }
            tuple += ")";
            return tuple;
        }
        case SemanticExpressionKind::InitializerList: {
            std::string list = "{";
            for (size_t i = 0; i < expression.children.size(); ++i) {
                if (i > 0) list += ", ";
                list += emitExpression(expression.children[i], context);
            }
            list += "}";
            return list;
        }
        case SemanticExpressionKind::Raw:
        default:
            return expression.text;
    }
}

static void appendLine(std::string& output, CppEmitContext& context, const std::string& line) {
    output += line;
    output += "\n";
    ++context.current_cpp_line;
}

static void emitStatementRef(
    std::string& output,
    CppEmitContext& context,
    const SemanticStatementRef& ref,
    const std::vector<SemanticVariableDeclarationIR>& vars,
    const std::vector<SemanticAssignmentIR>& assigns,
    const std::vector<SemanticTypeDefinitionIR>& types,
    const std::vector<SemanticStructDefinitionIR>& structs,
    const std::vector<SemanticEnumDefinitionIR>& enums,
    const std::vector<SemanticCallIR>& calls,
    const std::vector<SemanticReturnIR>& returns,
    const std::vector<SemanticIfIR>& ifs,
    const std::vector<SemanticWhileIR>& whiles,
    const std::vector<SemanticForIR>& fors,
    size_t indent
);

static std::string emitStructInitializerExpression(const SemanticExpressionIR& expression, CppEmitContext& context) {
    if (expression.kind != SemanticExpressionKind::InitializerList) {
        return emitExpression(expression, context);
    }

    std::string list = "{";
    for (size_t i = 0; i < expression.children.size(); ++i) {
        if (i > 0) {
            list += ", ";
        }

        const std::string entry = trim(expression.children[i].text);
        const size_t eq_pos = entry.find('=');
        if (eq_pos != std::string::npos) {
            const std::string lhs = sanitizeIdentifier(trim(entry.substr(0, eq_pos)));
            const std::string rhs = trim(entry.substr(eq_pos + 1));
            list += "." + lhs + " = " + rhs;
        } else {
            list += emitExpression(expression.children[i], context);
        }
    }
    list += "}";
    return list;
}

static std::string emitVariableInitializer(const SemanticVariableDeclarationIR& var, CppEmitContext& context) {
    if (var.initializer.kind == SemanticExpressionKind::InitializerList
        && var.type.kind == SemanticTypeKind::Named
        && context.struct_types.find(var.type.name) != context.struct_types.end()) {
        return emitStructInitializerExpression(var.initializer, context);
    }

    return emitExpression(var.initializer, context);
}

static std::string indent(size_t depth) {
    return std::string(depth * 4, ' ');
}

static std::string emitForClauseExpression(const SemanticExpressionIR& expression, CppEmitContext& context) {
    if (expression.kind != SemanticExpressionKind::Raw) {
        return emitExpression(expression, context);
    }

    const std::string text = trim(expression.text);
    for (const std::string op : {"+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "~=", "<<=", ">>="}) {
        if (text.find(op) != std::string::npos) {
            return text;
        }
    }

    for (const std::string op : {"+", "-", "*", "/", "%", "&", "|", "^", "~", "<<", ">>"}) {
        const std::string split_assignment = " " + op + " = ";
        const size_t split_pos = text.find(split_assignment);
        if (split_pos != std::string::npos) {
            return trim(text.substr(0, split_pos)) + " " + op + "= " + trim(text.substr(split_pos + split_assignment.size()));
        }
    }

    const size_t equal_pos = text.find('=');
    if (equal_pos == std::string::npos) {
        return emitExpression(expression, context);
    }

    std::string lhs = trim(text.substr(0, equal_pos));
    const std::string rhs = trim(text.substr(equal_pos + 1));
    bool is_mutable = false;

    if (lhs.rfind("mutable ", 0) == 0) {
        is_mutable = true;
        lhs = trim(lhs.substr(8));
    } else if (lhs.rfind("const ", 0) == 0) {
        lhs = trim(lhs.substr(6));
    }

    std::istringstream lhs_stream(lhs);
    std::vector<std::string> parts{};
    std::string part;
    while (lhs_stream >> part) {
        parts.push_back(part);
    }

    if (parts.size() < 2) {
        return emitExpression(expression, context);
    }

    const std::string name = parts.back();
    std::string type_name;
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        if (!type_name.empty()) {
            type_name += " ";
        }
        type_name += parts[i];
    }

    const SemanticExpressionIR initializer = parseExpressionIR(rhs, expression.location);
    const std::string qualifier = is_mutable ? "" : "const ";
    return qualifier + emitTypeRef(parseTypeRef(type_name), context) + " " + sanitizeIdentifier(name) + " = " + emitExpression(initializer, context);
}

static void emitStatementList(
    std::string& output,
    CppEmitContext& context,
    const std::vector<SemanticStatementRef>& order,
    const std::vector<SemanticVariableDeclarationIR>& vars,
    const std::vector<SemanticAssignmentIR>& assigns,
    const std::vector<SemanticTypeDefinitionIR>& types,
    const std::vector<SemanticStructDefinitionIR>& structs,
    const std::vector<SemanticEnumDefinitionIR>& enums,
    const std::vector<SemanticCallIR>& calls,
    const std::vector<SemanticReturnIR>& returns,
    const std::vector<SemanticIfIR>& ifs,
    const std::vector<SemanticWhileIR>& whiles,
    const std::vector<SemanticForIR>& fors,
    size_t indent_depth
) {
    for (const auto& ref : order) {
        emitStatementRef(output, context, ref, vars, assigns, types, structs, enums, calls, returns, ifs, whiles, fors, indent_depth);
    }
}

static void emitStatementRef(
    std::string& output,
    CppEmitContext& context,
    const SemanticStatementRef& ref,
    const std::vector<SemanticVariableDeclarationIR>& vars,
    const std::vector<SemanticAssignmentIR>& assigns,
    const std::vector<SemanticTypeDefinitionIR>& types,
    const std::vector<SemanticStructDefinitionIR>& structs,
    const std::vector<SemanticEnumDefinitionIR>& enums,
    const std::vector<SemanticCallIR>& calls,
    const std::vector<SemanticReturnIR>& returns,
    const std::vector<SemanticIfIR>& ifs,
    const std::vector<SemanticWhileIR>& whiles,
    const std::vector<SemanticForIR>& fors,
    size_t indent_depth
) {
    const std::string i = indent(indent_depth);

    switch (ref.kind) {
        case SemanticStatementKind::VariableDeclaration: {
            const auto& var = vars[ref.index];
            context.pushLine(var.location, "var");
            const std::string qualifier = var.is_mutable ? "" : "const ";
            appendLine(output, context, i + qualifier + emitTypeRef(var.type, context) + " " + sanitizeIdentifier(var.name) + " = " + emitVariableInitializer(var, context) + ";");
            break;
        }
        case SemanticStatementKind::Assignment: {
            const auto& asg = assigns[ref.index];
            context.pushLine(asg.location, "assignment");
            appendLine(output, context, i + sanitizeIdentifier(asg.target) + " " + asg.operator_symbol + " " + emitExpression(asg.expression, context) + ";");
            break;
        }
        case SemanticStatementKind::TypeDefinition: {
            const auto& type = types[ref.index];
            context.type_aliases[type.name] = type.base_type;
            context.pushLine(type.location, "type");
            appendLine(output, context, i + "using " + sanitizeIdentifier(type.name) + " = " + emitTypeRef(type.base_type, context) + ";");
            break;
        }
        case SemanticStatementKind::StructDefinition: {
            const auto& st = structs[ref.index];
            context.struct_types.insert(st.name);
            context.pushLine(st.location, "struct");
            appendLine(output, context, i + "struct " + sanitizeIdentifier(st.name) + " {");
            for (const auto& field : st.fields) {
                appendLine(output, context, i + "    " + emitTypeRef(field.type, context) + " " + sanitizeIdentifier(field.name) + ";");
            }
            appendLine(output, context, i + "};");
            break;
        }
        case SemanticStatementKind::EnumDefinition: {
            const auto& en = enums[ref.index];
            context.pushLine(en.location, "enum");
            appendLine(output, context, i + "enum class " + sanitizeIdentifier(en.name) + " {");
            for (size_t idx = 0; idx < en.values.size(); ++idx) {
                const std::string suffix = (idx + 1 < en.values.size()) ? "," : "";
                appendLine(output, context, i + "    " + sanitizeIdentifier(en.values[idx]) + suffix);
            }
            appendLine(output, context, i + "};");
            break;
        }
        case SemanticStatementKind::Call: {
            const auto& call = calls[ref.index];
            context.pushLine(call.location, "call");
            if (call.name == "print" || call.name == "println") {
                context.required_headers.insert("<iostream>");
                std::string rendered = "std::cout";
                for (const auto& arg : call.arguments) {
                    rendered += " << " + emitExpression(arg, context);
                }
                if (call.name == "println") {
                    rendered += " << std::endl";
                }
                appendLine(output, context, i + rendered + ";");
            } else {
                std::string rendered = sanitizeIdentifier(call.name) + "(";
                for (size_t idx = 0; idx < call.arguments.size(); ++idx) {
                    if (idx > 0) rendered += ", ";
                    rendered += emitExpression(call.arguments[idx], context);
                }
                rendered += ");";
                appendLine(output, context, i + rendered);
            }
            break;
        }
        case SemanticStatementKind::Return: {
            const auto& ret = returns[ref.index];
            context.pushLine(ret.location, "return");
            if (ret.expression.text.empty()) {
                appendLine(output, context, i + "return;");
            } else {
                appendLine(output, context, i + "return " + emitExpression(ret.expression, context) + ";");
            }
            break;
        }
        case SemanticStatementKind::Break:
            appendLine(output, context, i + "break;");
            break;
        case SemanticStatementKind::Continue:
            appendLine(output, context, i + "continue;");
            break;
        case SemanticStatementKind::If: {
            const auto& node = ifs[ref.index];
            context.pushLine(node.location, "if");
            appendLine(output, context, i + "if (" + emitExpression(node.condition, context) + ") {");
            emitStatementList(output, context, node.body, vars, assigns, types, structs, enums, calls, returns, ifs, whiles, fors, indent_depth + 1);
            appendLine(output, context, i + "}");
            break;
        }
        case SemanticStatementKind::While: {
            const auto& node = whiles[ref.index];
            context.pushLine(node.location, "while");
            appendLine(output, context, i + "while (" + emitExpression(node.condition, context) + ") {");
            emitStatementList(output, context, node.body, vars, assigns, types, structs, enums, calls, returns, ifs, whiles, fors, indent_depth + 1);
            appendLine(output, context, i + "}");
            break;
        }
        case SemanticStatementKind::For: {
            const auto& node = fors[ref.index];
            context.pushLine(node.location, "for");
            appendLine(output, context, i + "for (" + emitForClauseExpression(node.initializer, context) + "; " + emitForClauseExpression(node.condition, context) + "; " + emitForClauseExpression(node.update, context) + ") {");
            emitStatementList(output, context, node.body, vars, assigns, types, structs, enums, calls, returns, ifs, whiles, fors, indent_depth + 1);
            appendLine(output, context, i + "}");
            break;
        }
    }
}

CppEmitResult emitCpp(const SemanticProgram& program) {
    CppEmitResult result{};
    CppEmitContext context{};
    const std::vector<SemanticReturnIR> no_top_level_returns{};

    std::string body;

    for (const auto& statement : program.top_level_order) {
        emitStatementRef(
            body,
            context,
            statement,
            program.global_variables,
            program.global_assignments,
            program.type_definitions,
            program.struct_definitions,
            program.enum_definitions,
            program.calls,
            no_top_level_returns,
            program.if_statements,
            program.while_statements,
            program.for_statements,
            0
        );
    }

    for (const auto& function : program.functions) {
        context.pushLine(function.location, "function");

        const bool is_main = function.name == "main";
        const std::string return_type = is_main ? "int" : emitTypeRef(function.return_type, context);
        std::string signature = return_type + " " + sanitizeIdentifier(function.name) + "(";
        for (size_t i = 0; i < function.parameters.size(); ++i) {
            if (i > 0) signature += ", ";
            signature += emitTypeRef(function.parameters[i].type, context) + " " + sanitizeIdentifier(function.parameters[i].name);
        }
        signature += ")";

        appendLine(body, context, signature + " {");
        emitStatementList(
            body,
            context,
            function.body_order,
            function.variable_declarations,
            function.assignments,
            function.type_definitions,
            function.struct_definitions,
            function.enum_definitions,
            function.calls,
            function.returns,
            function.if_statements,
            function.while_statements,
            function.for_statements,
            1
        );
        if (is_main) {
            appendLine(body, context, "    return 0;");
        }
        appendLine(body, context, "}");
    }

    std::string output;
    output += "// Generated by C-Prime transpiler\n";

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
