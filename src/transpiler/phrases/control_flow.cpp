#pragma once

#include "../statement_data.cpp"

static void emitBreakStatement(std::string& output, CppEmitContext& context, const std::string& indent_str) {
    appendLine(output, context, indent_str + "break;");
}

static void emitContinueStatement(std::string& output, CppEmitContext& context, const std::string& indent_str) {
    appendLine(output, context, indent_str + "continue;");
}

static void emitIfStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticIfIR& node,
    const CppEmitStatementData& data,
    const std::string& indent_str,
    size_t indent_depth
) {
    context.pushLine(node.location, "if");
    appendLine(output, context, indent_str + "if (" + emitExpression(node.condition, context) + ") {");
    emitStatementList(output, context, node.body, data, indent_depth + 1);
    appendLine(output, context, indent_str + "}");
}

static void emitElseStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticElseIR& node,
    const CppEmitStatementData& data,
    const std::string& indent_str,
    size_t indent_depth
) {
    context.pushLine(node.location, "else");
    if (node.condition.text.empty()) {
        appendLine(output, context, indent_str + "else {");
    } else {
        appendLine(output, context, indent_str + "else if (" + emitExpression(node.condition, context) + ") {");
    }
    emitStatementList(output, context, node.body, data, indent_depth + 1);
    appendLine(output, context, indent_str + "}");
}

static void emitWhileStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticWhileIR& node,
    const CppEmitStatementData& data,
    const std::string& indent_str,
    size_t indent_depth
) {
    context.pushLine(node.location, "while");
    appendLine(output, context, indent_str + "while (" + emitExpression(node.condition, context) + ") {");
    emitStatementList(output, context, node.body, data, indent_depth + 1);
    appendLine(output, context, indent_str + "}");
}

static void emitForStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticForIR& node,
    const CppEmitStatementData& data,
    const std::string& indent_str,
    size_t indent_depth
) {
    context.pushLine(node.location, "for");
    appendLine(output, context, indent_str + "for (" + emitForClauseExpression(node.initializer, context) + "; " + emitForClauseExpression(node.condition, context) + "; " + emitForClauseExpression(node.update, context) + ") {");
    emitStatementList(output, context, node.body, data, indent_depth + 1);
    appendLine(output, context, indent_str + "}");
}
