#pragma once

#include "../statement_data.cpp"

static void emitReturnStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticReturnIR& ret,
    const std::string& indent_str
) {
    context.pushLine(ret.location, "return");
    if (ret.expression.text.empty()) {
        appendLine(output, context, indent_str + "return;");
    } else {
        if (context.current_function_has_ensures && !context.current_return_value_name.empty()) {
            const std::string return_name = context.current_return_value_name;
            appendLine(output, context, indent_str + "const auto " + return_name + " = " + emitExpression(ret.expression, context) + ";");
            context.required_headers.insert("<stdexcept>");
            appendLine(output, context, indent_str + "if (!(" + emitExpression(context.current_function_ensures_clause, context) + ")) { throw std::runtime_error(\"function ensures clause violated\"); }");
            appendLine(output, context, indent_str + "return " + return_name + ";");
        } else {
            appendLine(output, context, indent_str + "return " + emitExpression(ret.expression, context) + ";");
        }
    }
}
