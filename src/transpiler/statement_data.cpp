#pragma once

#include "expression_emitter.cpp"

// Bundles per-kind statement storage so phrase emitters avoid long parameter lists.
struct CppEmitStatementData {
    const std::vector<SemanticVariableDeclarationIR>& vars;
    const std::vector<SemanticAssignmentIR>& assigns;
    const std::vector<SemanticTypeDefinitionIR>& types;
    const std::vector<SemanticStructDefinitionIR>& structs;
    const std::vector<SemanticEnumDefinitionIR>& enums;
    const std::vector<SemanticImportIR>& imports;
    const std::vector<SemanticCallIR>& calls;
    const std::vector<SemanticReturnIR>& returns;
    const std::vector<SemanticIfIR>& ifs;
    const std::vector<SemanticWhileIR>& whiles;
    const std::vector<SemanticForIR>& fors;
    const std::vector<SemanticElseIR>& elses;
    const std::vector<SemanticTriviaIR>& trivia;
};

static void emitStatementList(
    std::string& output,
    CppEmitContext& context,
    const std::vector<SemanticStatementRef>& order,
    const CppEmitStatementData& data,
    size_t indent_depth
);
