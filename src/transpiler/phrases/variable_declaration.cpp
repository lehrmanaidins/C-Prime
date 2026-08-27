#pragma once

#include "../statement_data.cpp"

static void emitVariableDeclarationStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticVariableDeclarationIR& var,
    const std::string& indent_str
) {
    context.pushLine(var.location, "var");
    const std::string qualifier = var.is_mutable ? "" : "const ";
    appendLine(output, context, indent_str + qualifier + emitTypeRef(var.type, context) + " " + sanitizeIdentifier(var.name) + " = " + emitVariableInitializer(var, context) + ";");
}
