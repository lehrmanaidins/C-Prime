#pragma once

#include "../statement_data.cpp"

static void emitReturnStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticReturnIR& ret,
    const std::string& indent_str
) {
    context.pushLine(ret.location, "return");
    if (ret.expression.text.empty()) {
        appendLine(output, context, indent_str + "return;");
    } else {
        appendLine(output, context, indent_str + "return " + emitExpression(ret.expression, context) + ";");
    }
}
