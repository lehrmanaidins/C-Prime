#pragma once

#include "../statement_data.cpp"

static void emitTypeDefinitionStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticTypeDefinitionIR& type_def,
    const std::string& indent_str
) {
    context.type_aliases[type_def.name] = type_def.base_type;
    context.pushLine(type_def.location, "type");
    appendLine(output, context, indent_str + "using " + sanitizeIdentifier(type_def.name) + " = " + emitTypeRef(type_def.base_type, context) + ";");
}
