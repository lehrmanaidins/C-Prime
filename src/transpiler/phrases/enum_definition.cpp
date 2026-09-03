#pragma once

#include "../statement_data.cpp"

static void emitEnumDefinitionStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticEnumDefinitionIR& en,
    const std::string& indent_str
) {
    context.pushLine(en.location, "enum");
    context.required_headers.insert("<cstdint>");
    appendLine(output, context, indent_str + "enum class " + en.name + " : " + en.underlying_cpp_type + " {");
    for (size_t idx = 0; idx < en.values.size(); ++idx) {
        const std::string suffix = (idx + 1 < en.values.size()) ? "," : "";
        appendLine(output, context, indent_str + "    " + en.values[idx] + suffix);
    }
    appendLine(output, context, indent_str + "};");
}
