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

// A `limit (N)` clause lowers to a hidden 64-bit counter declared in an extra
// scope around the loop, plus an `if (counter++ >= (N)) break;` injected as the
// first statement of the loop body. `emitLoopLimitPrologue` opens that scope and
// returns the counter name (empty when there is no limit); the caller emits the
// guard as the body's first line and calls `emitLoopLimitEpilogue` to close it.
static std::string emitLoopLimitPrologue(
    std::string& output,
    CppEmitContext& context,
    const std::string& limit,
    const std::string& indent_str
) {
    if (limit.empty()) {
        return "";
    }

    context.required_headers.insert("<cstddef>");
    const std::string counter = "cprime_loop_limit_counter_" + std::to_string(context.loop_serial);
    const std::string limit_max = "cprime_loop_limit_max_" + std::to_string(context.loop_serial);
    context.loop_serial += 1;
    appendLine(output, context, indent_str + "{");
    appendLine(output, context, indent_str + "    std::size_t " + counter + " = 0;");
    appendLine(output, context, indent_str + "    const std::size_t " + limit_max + " = " + limit + ";");
    return counter;
}

static void emitLoopLimitGuard(
    std::string& output,
    CppEmitContext& context,
    const std::string& counter,
    const std::string& limit,
    const std::string& body_indent_str
) {
    if (counter.empty()) {
        return;
    }
    const std::string limit_max = "cprime_loop_limit_max_" + counter.substr(counter.find_last_of('_') + 1);

    appendLine(output, context, body_indent_str + "if (" + counter + "++ >= (" + limit_max + ")) { break; }");
}

static void emitLoopLimitEpilogue(
    std::string& output,
    CppEmitContext& context,
    const std::string& counter,
    const std::string& indent_str
) {
    if (!counter.empty()) {
        appendLine(output, context, indent_str + "}");
    }
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
    const std::string counter = emitLoopLimitPrologue(output, context, node.limit, indent_str);
    const std::string limit_max = "cprime_loop_limit_max_" + std::to_string(context.loop_serial - 1);
    const size_t loop_depth = indent_depth + (counter.empty() ? 0 : 1);
    const std::string loop_indent = indent(loop_depth);
    const std::string body_indent = indent(loop_depth + 1);

    switch (node.loop_kind) {
        case SemanticLoopKind::Endless:
            appendLine(output, context, loop_indent + "while (true) {");
            break;
        case SemanticLoopKind::DoWhile:
            appendLine(output, context, loop_indent + "do {");
            break;
        case SemanticLoopKind::While:
            appendLine(output, context, loop_indent + "while (" + emitExpression(node.condition, context) + ") {");
            break;
    }

    emitLoopLimitGuard(output, context, counter, limit_max, body_indent);
    emitStatementList(output, context, node.body, data, loop_depth + 1);

    if (node.loop_kind == SemanticLoopKind::DoWhile) {
        appendLine(output, context, loop_indent + "} while (" + emitExpression(node.condition, context) + ");");
    } else {
        appendLine(output, context, loop_indent + "}");
    }
    emitLoopLimitEpilogue(output, context, counter, indent_str);
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
    const std::string counter = emitLoopLimitPrologue(output, context, node.limit, indent_str);
    const std::string limit_max = "cprime_loop_limit_max_" + std::to_string(context.loop_serial - 1);
    const size_t loop_depth = indent_depth + (counter.empty() ? 0 : 1);
    const std::string loop_indent = indent(loop_depth);
    const std::string body_indent = indent(loop_depth + 1);

    if (node.is_foreach) {
        const std::string element_type = emitTypeRef(parseTypeRef(node.foreach_type), context);
        appendLine(output, context, loop_indent + "for (" + element_type + " " + node.foreach_name
            + " : " + emitExpression(node.initializer, context) + ") {");
    } else {
        appendLine(output, context, loop_indent + "for (" + emitForClauseExpression(node.initializer, context)
            + "; " + emitForClauseExpression(node.condition, context)
            + "; " + emitForClauseExpression(node.update, context) + ") {");
    }

    emitLoopLimitGuard(output, context, counter, limit_max, body_indent);
    emitStatementList(output, context, node.body, data, loop_depth + 1);
    appendLine(output, context, loop_indent + "}");
    emitLoopLimitEpilogue(output, context, counter, indent_str);
}
