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
    std::string declarator;
    if (var.type.kind == SemanticTypeKind::Pointer) {
        // Immutability governs the pointer itself, so 'const' is postfixed rather than prefixed.
        declarator = type_str + (var.is_mutable ? "" : " const") + " " + sanitizeIdentifier(var.name);
    } else {
        const std::string qualifier = var.is_mutable ? "" : "const ";
        declarator = qualifier + type_str + " " + sanitizeIdentifier(var.name);
    }
    appendLine(output, context, indent_str + declarator + " = " + emitVariableInitializer(var, context) + ";");
}
