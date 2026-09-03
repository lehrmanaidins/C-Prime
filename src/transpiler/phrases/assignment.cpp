#pragma once

#include "../statement_data.cpp"

static void emitAssignmentStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticAssignmentIR& asg,
    const std::string& indent_str
) {
    context.pushLine(asg.location, "assignment");
    appendLine(output, context, indent_str + asg.target + " " + asg.operator_symbol + " " + emitExpression(asg.expression, context) + ";");
}
