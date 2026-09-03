#pragma once

#include "../statement_data.cpp"

static std::string sanitizeDottedIdentifier(const std::string& name) {
    std::string result;
    size_t start = 0;
    while (start <= name.size()) {
        const size_t dot = name.find('.', start);
        const std::string segment = name.substr(start, dot == std::string::npos ? std::string::npos : dot - start);
        if (!result.empty()) {
            result += ".";
        }
        result += segment;
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }
    return result;
}

static void emitCallStatement(
    std::string& output,
    CppEmitContext& context,
    const SemanticCallIR& call,
    const std::string& indent_str
) {
    context.pushLine(call.location, "call");
    std::string rendered = sanitizeDottedIdentifier(call.name) + "(";
    for (size_t idx = 0; idx < call.arguments.size(); ++idx) {
        if (idx > 0) rendered += ", ";
        rendered += emitExpression(call.arguments[idx], context);
    }
    rendered += ");";
    appendLine(output, context, indent_str + rendered);
}
