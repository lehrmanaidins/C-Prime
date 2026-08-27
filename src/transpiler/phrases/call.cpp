#pragma once

#include "../statement_data.cpp"

static void emitCallStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticCallIR& call,
    const std::string& indent_str
) {
    context.pushLine(call.location, "call");
    std::string rendered = sanitizeIdentifier(call.name) + "(";
    for (size_t idx = 0; idx < call.arguments.size(); ++idx) {
        if (idx > 0) rendered += ", ";
        rendered += emitExpression(call.arguments[idx], context);
    }
    rendered += ");";
    appendLine(output, context, indent_str + rendered);
}
