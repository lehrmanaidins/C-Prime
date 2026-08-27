#pragma once

#include "../statement_data.cpp"

static void emitImportStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticImportIR& import_ir
) {
    context.pushLine(import_ir.location, "import");
    appendLine(output, context, "#include \"" + import_ir.path + "\"");
}
