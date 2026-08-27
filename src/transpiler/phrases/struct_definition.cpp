#pragma once

#include "../statement_data.cpp"

static void emitStructDefinitionStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticStructDefinitionIR& st,
    const std::string& indent_str
) {
    context.struct_types.insert(st.name);
    context.pushLine(st.location, "struct");
    appendLine(output, context, indent_str + "struct " + sanitizeIdentifier(st.name) + " {");
    for (const auto& field : st.fields) {
        appendLine(output, context, indent_str + "    " + emitTypeRef(field.type, context) + " " + sanitizeIdentifier(field.name) + ";");
    }
    appendLine(output, context, indent_str + "};");
}
