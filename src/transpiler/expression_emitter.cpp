#pragma once

#include <sstream>

#include "type_emitter.cpp"

static std::string emitExpression(const SemanticExpressionIR& expression, CppEmitContext& context) {
    switch (expression.kind) {
        case SemanticExpressionKind::Identifier:
            return expression.text;
        case SemanticExpressionKind::Literal:
            // Character (and every other) literal is emitted verbatim.
            return expression.text;
        case SemanticExpressionKind::Binary:
            if (expression.children.size() == 2) {
                return "(" + emitExpression(expression.children[0], context) + " " + expression.operator_symbol + " " + emitExpression(expression.children[1], context) + ")";
            }
            return expression.text;
        case SemanticExpressionKind::Call: {
            std::string callee = expression.operator_symbol;
            if (expression.operator_symbol == "pointer" || expression.operator_symbol == "reference") {
                context.required_headers.insert("\"memory.hpp\"");
                callee = "cprime::" + expression.operator_symbol;
            }
            std::string call = callee + "(";
            for (size_t i = 0; i < expression.children.size(); ++i) {
                if (i > 0) call += ", ";
                call += emitExpression(expression.children[i], context);
            }
            call += ")";
            return call;
        }
        case SemanticExpressionKind::MemberAccess:
            if (expression.children.size() == 2) {
                return emitExpression(expression.children[0], context) + "." + emitExpression(expression.children[1], context);
            }
            return expression.text;
        case SemanticExpressionKind::IndexAccess:
            if (expression.children.size() == 2) {
                return emitExpression(expression.children[0], context) + "[" + emitExpression(expression.children[1], context) + "]";
            }
            return expression.text;
        case SemanticExpressionKind::QualifiedName:
            if (expression.children.size() == 2) {
                return emitExpression(expression.children[0], context) + "::" + emitExpression(expression.children[1], context);
            }
            return expression.text;
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
            const std::string lhs = trim(entry.substr(0, eq_pos));
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
    if (var.initializer.kind == SemanticExpressionKind::InitializerList) {
        for (const auto& child : var.initializer.children) {
            if (child.text.find('=') != std::string::npos) {
                return emitStructInitializerExpression(var.initializer, context);
            }
        }
    }

    if (var.initializer.kind == SemanticExpressionKind::InitializerList
        && var.type.kind == SemanticTypeKind::Named
        && context.struct_types.find(var.type.name) != context.struct_types.end()) {
        return emitStructInitializerExpression(var.initializer, context);
    }

    return emitExpression(var.initializer, context);
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
    return qualifier + emitTypeRef(parseTypeRef(type_name), context) + " " + name + " = " + emitExpression(initializer, context);
}
