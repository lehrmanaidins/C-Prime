#pragma once

#include "../statement_data.cpp"

static std::string emitGenericParameterList(const std::vector<SemanticGenericParameterIR>& parameters) {
    if (parameters.empty()) {
        return "";
    }
    std::string line = "template <";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) line += ", ";
        line += "typename " + parameters[i].name;
    }
    return line + ">";
}

static void emitStructDefinitionStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticStructDefinitionIR& st,
    const std::string& indent_str
) {
    context.struct_types.insert(st.name);
    context.pushLine(st.location, "struct");
    const std::string template_line = emitGenericParameterList(st.template_parameters);
    if (!template_line.empty()) {
        appendLine(output, context, indent_str + template_line);
    }
    appendLine(output, context, indent_str + "struct " + sanitizeIdentifier(st.name) + " {");
    for (const auto& field : st.fields) {
        appendLine(output, context, indent_str + "    " + emitTypeRef(field.type, context) + " " + sanitizeIdentifier(field.name) + ";");
    }
    appendLine(output, context, indent_str + "};");
}
