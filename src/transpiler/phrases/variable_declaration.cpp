#pragma once

#include "../statement_data.cpp"

static void emitVariableDeclarationStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticVariableDeclarationIR& var,
    const std::string& indent_str
) {
    context.pushLine(var.location, "var");
    const std::string type_str = emitTypeRef(var.type, context);
    const std::string qualifier = var.is_mutable ? "" : "const ";
    const std::string declarator = qualifier + type_str + " " + var.name;
    appendLine(output, context, indent_str + declarator + " = " + emitVariableInitializer(var, context) + ";");
}
