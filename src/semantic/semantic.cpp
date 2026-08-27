#pragma once

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../parser/parsed_phrase.cpp"
#include "../parser/phrases/assignment.cpp"
#include "../parser/phrases/control_flow.cpp"
#include "../parser/phrases/enum_definition.cpp"
#include "../parser/phrases/enum_value_definition.cpp"
#include "../parser/phrases/function.cpp"
#include "../parser/phrases/parameter_declaration.cpp"
#include "../parser/phrases/parameter_definition.cpp"
#include "../parser/phrases/return_statement.cpp"
#include "../parser/phrases/statement.cpp"
#include "../parser/phrases/struct_definition.cpp"
#include "../parser/phrases/type_definition.cpp"
#include "../parser/phrases/variable_declaration.cpp"
#include "expression.cpp"
#include "type_ref.cpp"

struct SemanticParameterIR {
    std::string name;
    SemanticTypeRef type;
    SourceLocation location;
};

struct SemanticVariableDeclarationIR {
    std::string name;
    SemanticTypeRef type;
    SemanticExpressionIR initializer;
    bool is_mutable;
    SourceLocation location;
};

struct SemanticAssignmentIR {
    std::string target;
    std::string operator_symbol;
    SemanticExpressionIR expression;
    SourceLocation location;
};

struct SemanticTypeDefinitionIR {
    std::string name;
    SemanticTypeRef base_type;
    SourceLocation location;
};

struct SemanticStructFieldIR {
    std::string name;
    SemanticTypeRef type;
    SourceLocation location;
};

struct SemanticStructDefinitionIR {
    std::string name;
    std::vector<SemanticStructFieldIR> fields;
    SourceLocation location;
};

struct SemanticEnumDefinitionIR {
    std::string name;
    std::vector<std::string> values;
    SourceLocation location;
};

struct SemanticCallIR {
    std::string name;
    std::vector<SemanticExpressionIR> arguments;
    SourceLocation location;
};

struct SemanticReturnIR {
    SemanticExpressionIR expression;
    SourceLocation location;
};

struct SemanticIfIR;
struct SemanticWhileIR;
struct SemanticForIR;

enum class SemanticStatementKind {
    VariableDeclaration,
    Assignment,
    TypeDefinition,
    StructDefinition,
    EnumDefinition,
    If,
    While,
    For,
    Else,
    Call,
    Return,
    Break,
    Continue
};

struct SemanticStatementRef {
    SemanticStatementKind kind;
    size_t index;
};

struct SemanticIfIR {
    SemanticExpressionIR condition;
    std::vector<SemanticStatementRef> body;
    SourceLocation location;
};

struct SemanticWhileIR {
    SemanticExpressionIR condition;
    std::vector<SemanticStatementRef> body;
    SourceLocation location;
};

struct SemanticForIR {
    SemanticExpressionIR initializer;
    SemanticExpressionIR condition;
    SemanticExpressionIR update;
    std::vector<SemanticStatementRef> body;
    SourceLocation location;
};

struct SemanticElseIR {
    SemanticExpressionIR condition;
    std::vector<SemanticStatementRef> body;
    SourceLocation location;
};

struct SemanticFunctionIR {
    std::string name;
    SemanticTypeRef return_type;
    std::vector<SemanticParameterIR> parameters;

    std::vector<SemanticVariableDeclarationIR> variable_declarations;
    std::vector<SemanticAssignmentIR> assignments;
    std::vector<SemanticTypeDefinitionIR> type_definitions;
    std::vector<SemanticStructDefinitionIR> struct_definitions;
    std::vector<SemanticEnumDefinitionIR> enum_definitions;
    std::vector<SemanticCallIR> calls;
    std::vector<SemanticReturnIR> returns;
    std::vector<SemanticIfIR> if_statements;
    std::vector<SemanticWhileIR> while_statements;
    std::vector<SemanticForIR> for_statements;
    std::vector<SemanticElseIR> else_statements;
    std::vector<SemanticStatementRef> body_order;

    SourceLocation location;
};

struct SemanticProgram {
    std::vector<SemanticFunctionIR> functions;
    std::vector<SemanticVariableDeclarationIR> global_variables;
    std::vector<SemanticAssignmentIR> global_assignments;
    std::vector<SemanticTypeDefinitionIR> type_definitions;
    std::vector<SemanticStructDefinitionIR> struct_definitions;
    std::vector<SemanticEnumDefinitionIR> enum_definitions;
    std::vector<SemanticCallIR> calls;
    std::vector<SemanticIfIR> if_statements;
    std::vector<SemanticWhileIR> while_statements;
    std::vector<SemanticForIR> for_statements;
    std::vector<SemanticElseIR> else_statements;
    std::vector<SemanticStatementRef> top_level_order;
};

enum class TypeSymbolKind {
    Primitive,
    Domain,
    Struct,
    Enum
};

struct TypeSymbol {
    TypeSymbolKind kind;
    std::string underlying;
};

struct VariableSymbol {
    std::string type_name;
    bool is_mutable;
};

struct SemanticSymbolTable {
    std::unordered_map<std::string, TypeSymbol> known_types;
    std::unordered_set<std::string> known_functions;
    std::unordered_map<std::string, std::string> function_return_types;
    std::unordered_map<std::string, size_t> function_parameter_counts;
};

struct ValidationContext {
    const SemanticSymbolTable& symbols;
    std::vector<std::unordered_map<std::string, VariableSymbol>> variable_scopes;
    std::string current_function_return_type;
    size_t loop_depth = 0;
};

static SourceLocation phraseStartLocation(const std::shared_ptr<ParsedPhrase>& phrase) {
    if (!phrase || !phrase->source_phrase) {
        return SourceLocation{0, 0};
    }

    return SourceLocation{phrase->source_phrase->start_line, phrase->source_phrase->start_column};
}

static std::string joinPhraseText(const std::shared_ptr<ParsedPhrase>& phrase) {
    if (!phrase || !phrase->source_phrase) {
        return "";
    }

    std::string text;
    for (const auto& token : phrase->source_phrase->tokens) {
        if (!token) {
            continue;
        }

        if (!text.empty()) {
            text += " ";
        }
        text += token->value;
    }

    return text;
}

static std::string phraseLocation(const std::shared_ptr<ParsedPhrase>& phrase) {
    if (!phrase || !phrase->source_phrase || phrase->source_phrase->start_line == 0) {
        return "<unknown>";
    }

    return std::to_string(phrase->source_phrase->start_line)
        + ":"
        + std::to_string(phrase->source_phrase->start_column);
}

static std::runtime_error semanticError(const std::shared_ptr<ParsedPhrase>& phrase, const std::string& message) {
    return std::runtime_error("Semantic error at " + phraseLocation(phrase) + ": " + message);
}

static bool isKnownType(const SemanticSymbolTable& symbols, const std::string& raw_type) {
    const std::string type = normalizeTypeName(raw_type);
    if (type.empty()) {
        return false;
    }

    return symbols.known_types.find(type) != symbols.known_types.end();
}

static bool isKnownTypeRef(const SemanticSymbolTable& symbols, const SemanticTypeRef& type_ref) {
    if (type_ref.kind == SemanticTypeKind::Tuple) {
        for (const auto& element : type_ref.tuple_elements) {
            if (!isKnownTypeRef(symbols, element)) {
                return false;
            }
        }
        return true;
    }

    if (type_ref.name.empty()) {
        return false;
    }

    return symbols.known_types.find(normalizeTypeName(type_ref.name)) != symbols.known_types.end();
}

static bool isKnownTypeRefString(const SemanticSymbolTable& symbols, const std::string& raw_type) {
    return isKnownTypeRef(symbols, parseTypeRef(raw_type));
}

static void enterScope(ValidationContext& context) {
    context.variable_scopes.emplace_back();
}

static void leaveScope(ValidationContext& context) {
    if (!context.variable_scopes.empty()) {
        context.variable_scopes.pop_back();
    }
}

static VariableSymbol* lookupVariable(ValidationContext& context, const std::string& name) {
    for (auto it = context.variable_scopes.rbegin(); it != context.variable_scopes.rend(); ++it) {
        auto symbol_it = it->find(name);
        if (symbol_it != it->end()) {
            return &symbol_it->second;
        }
    }

    return nullptr;
}

static void declareVariable(
    ValidationContext& context,
    const std::string& name,
    const std::string& type_name,
    bool is_mutable,
    const std::shared_ptr<ParsedPhrase>& phrase
) {
    if (context.variable_scopes.empty()) {
        enterScope(context);
    }

    auto& scope = context.variable_scopes.back();
    if (scope.find(name) != scope.end()) {
        throw semanticError(phrase, "duplicate declaration of variable '" + name + "' in phrase: " + joinPhraseText(phrase));
    }

    scope.insert({name, VariableSymbol{type_name, is_mutable}});
}

static void collectSymbolsFromPhrase(const std::shared_ptr<ParsedPhrase>& phrase, SemanticSymbolTable& symbols);

static void registerTypeSymbol(
    const std::shared_ptr<ParsedPhrase>& phrase,
    const std::string& type_name,
    TypeSymbol type_symbol,
    SemanticSymbolTable& symbols
) {
    const std::string normalized = normalizeTypeName(type_name);
    if (normalized.empty()) {
        throw semanticError(phrase, "empty type symbol");
    }

    if (symbols.known_types.find(normalized) != symbols.known_types.end()) {
        throw semanticError(phrase, "duplicate type declaration '" + normalized + "'");
    }

    symbols.known_types.insert({normalized, type_symbol});
}

static void collectSymbolsFromFunction(const std::shared_ptr<ParsedFunction>& function_phrase, SemanticSymbolTable& symbols) {
    if (!function_phrase) {
        return;
    }

    if (symbols.known_functions.find(function_phrase->name) != symbols.known_functions.end()) {
        throw semanticError(function_phrase, "duplicate function '" + function_phrase->name + "'");
    }

    symbols.known_functions.insert(function_phrase->name);
    symbols.function_return_types[function_phrase->name] = normalizeTypeName(function_phrase->return_type.empty() ? "void" : function_phrase->return_type);
    symbols.function_parameter_counts[function_phrase->name] = function_phrase->parameters.size();

    for (const auto& nested : function_phrase->nested_phrases) {
        collectSymbolsFromPhrase(nested, symbols);
    }
}

static void collectSymbolsFromPhrase(const std::shared_ptr<ParsedPhrase>& phrase, SemanticSymbolTable& symbols) {
    if (!phrase) {
        return;
    }

    switch (phrase->kind) {
        case ParsedPhraseKind::Function:
            collectSymbolsFromFunction(std::dynamic_pointer_cast<ParsedFunction>(phrase), symbols);
            break;
        case ParsedPhraseKind::TypeDefinition: {
            auto type_def = std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase);
            if (type_def) {
                registerTypeSymbol(
                    phrase,
                    type_def->name,
                    TypeSymbol{TypeSymbolKind::Domain, trim(type_def->base_type)},
                    symbols
                );
            }
            break;
        }
        case ParsedPhraseKind::StructDefinition: {
            auto struct_def = std::dynamic_pointer_cast<ParsedStructDefinition>(phrase);
            if (struct_def) {
                registerTypeSymbol(
                    phrase,
                    struct_def->name,
                    TypeSymbol{TypeSymbolKind::Struct, ""},
                    symbols
                );
            }
            for (const auto& nested : phrase->nested_phrases) {
                collectSymbolsFromPhrase(nested, symbols);
            }
            break;
        }
        case ParsedPhraseKind::EnumDefinition: {
            auto enum_def = std::dynamic_pointer_cast<ParsedEnumDefinition>(phrase);
            if (enum_def) {
                registerTypeSymbol(
                    phrase,
                    enum_def->name,
                    TypeSymbol{TypeSymbolKind::Enum, ""},
                    symbols
                );
            }
            for (const auto& nested : phrase->nested_phrases) {
                collectSymbolsFromPhrase(nested, symbols);
            }
            break;
        }
        default:
            for (const auto& nested : phrase->nested_phrases) {
                collectSymbolsFromPhrase(nested, symbols);
            }
            break;
    }
}

static std::optional<std::string> inferExpressionTypeName(
    const SemanticExpressionIR& expression,
    ValidationContext& context
) {
    switch (expression.kind) {
        case SemanticExpressionKind::Identifier: {
            VariableSymbol* symbol = lookupVariable(context, trim(expression.text));
            if (symbol) {
                return std::optional<std::string>{symbol->type_name};
            }
            return std::nullopt;
        }
        case SemanticExpressionKind::Literal:
            return inferLiteralTypeName(expression.text);
        case SemanticExpressionKind::TupleLiteral:
            return std::optional<std::string>{"tuple"};
        case SemanticExpressionKind::InitializerList:
            return std::optional<std::string>{"init_list"};
        case SemanticExpressionKind::Call:
            if (context.symbols.function_return_types.find(expression.operator_symbol) != context.symbols.function_return_types.end()) {
                return std::optional<std::string>{context.symbols.function_return_types.at(expression.operator_symbol)};
            }
            return std::nullopt;
        case SemanticExpressionKind::Binary:
            if (expression.operator_symbol == "==" || expression.operator_symbol == "!="
                || expression.operator_symbol == ">=" || expression.operator_symbol == "<="
                || expression.operator_symbol == ">" || expression.operator_symbol == "<"
                || expression.operator_symbol == "&&" || expression.operator_symbol == "||") {
                return std::optional<std::string>{"bool"};
            }
            if (!expression.children.empty()) {
                return inferExpressionTypeName(expression.children.front(), context);
            }
            return std::nullopt;
        case SemanticExpressionKind::Raw:
        default:
            return std::nullopt;
    }
}

static bool isNumericPrimitive(const std::string& type_name) {
    return type_name == "int8" || type_name == "int16" || type_name == "int32" || type_name == "int64"
        || type_name == "uint8" || type_name == "uint16" || type_name == "uint32" || type_name == "uint64"
        || type_name == "float32" || type_name == "float64";
}

static std::optional<std::string> expressionRootIdentifier(const SemanticExpressionIR& expression) {
    if (expression.kind == SemanticExpressionKind::Identifier) {
        return std::optional<std::string>{trim(expression.text)};
    }

    if ((expression.kind == SemanticExpressionKind::MemberAccess || expression.kind == SemanticExpressionKind::IndexAccess)
        && !expression.children.empty()) {
        return expressionRootIdentifier(expression.children.front());
    }

    return std::nullopt;
}

static bool isAssignableExpression(const SemanticExpressionIR& expression) {
    return expression.kind == SemanticExpressionKind::Identifier
        || expression.kind == SemanticExpressionKind::MemberAccess
        || expression.kind == SemanticExpressionKind::IndexAccess;
}

static void validateExpression(
    const std::shared_ptr<ParsedPhrase>& phrase,
    const SemanticExpressionIR& expression,
    ValidationContext& context
) {
    if (expression.kind == SemanticExpressionKind::Call) {
        if (context.symbols.known_functions.find(expression.operator_symbol) == context.symbols.known_functions.end()) {
            throw semanticError(phrase, "unknown function '" + expression.operator_symbol + "'");
        }

        if (expression.operator_symbol != "print" && expression.operator_symbol != "println") {
            auto arity = context.symbols.function_parameter_counts.find(expression.operator_symbol);
            if (arity != context.symbols.function_parameter_counts.end() && arity->second != expression.children.size()) {
                throw semanticError(phrase, "function '" + expression.operator_symbol + "' expects " + std::to_string(arity->second) + " argument(s), got " + std::to_string(expression.children.size()));
            }
        }
    }

    for (const auto& child : expression.children) {
        validateExpression(phrase, child, context);
    }
}

static void validateAssignmentCompatibility(
    const std::shared_ptr<ParsedPhrase>& phrase,
    const std::string& target_type,
    const SemanticExpressionIR& expression,
    ValidationContext& context
) {
    const std::string normalized_target = normalizeTypeName(target_type);
    auto target_it = context.symbols.known_types.find(normalized_target);
    if (target_it == context.symbols.known_types.end()) {
        return;
    }

    const TypeSymbol target_symbol = target_it->second;
    const std::optional<std::string> expression_type = inferExpressionTypeName(expression, context);

    if (target_symbol.kind == TypeSymbolKind::Domain) {
        if (!expression_type.has_value()) {
            return;
        }

        const std::string source_type = normalizeTypeName(expression_type.value());
        if (source_type == normalized_target) {
            return;
        }

        const std::string underlying = trim(target_symbol.underlying);
        const SemanticTypeRef underlying_type = parseTypeRef(underlying);
        if (expression.kind == SemanticExpressionKind::TupleLiteral
            && underlying_type.kind == SemanticTypeKind::Tuple) {
            return;
        }

        if (expression.kind == SemanticExpressionKind::InitializerList
            && !underlying_type.array_dimensions.empty()) {
            return;
        }

        if (expression.kind == SemanticExpressionKind::Literal) {
            return;
        }

        throw semanticError(
            phrase,
            "domain type '" + normalized_target + "' cannot be implicitly assigned from '" + source_type + "'"
        );
    }

    if (target_symbol.kind == TypeSymbolKind::Primitive) {
        if (!expression_type.has_value()) {
            return;
        }

        const std::string source_type = normalizeTypeName(expression_type.value());
        auto source_it = context.symbols.known_types.find(source_type);
        if (source_it != context.symbols.known_types.end() && source_it->second.kind == TypeSymbolKind::Domain) {
            throw semanticError(
                phrase,
                "primitive type '" + normalized_target + "' cannot be implicitly assigned from domain type '" + source_type + "'"
            );
        }

        if (source_type == normalized_target) {
            return;
        }

        if (isNumericPrimitive(normalized_target) && isNumericPrimitive(source_type)) {
            return;
        }
    }
}

static void analyzePhrase(const std::shared_ptr<ParsedPhrase>& phrase, ValidationContext& context);
static void analyzePhraseList(const std::vector<std::shared_ptr<ParsedPhrase>>& phrases, ValidationContext& context);

static void analyzeControlFlowBody(const std::shared_ptr<ParsedPhrase>& phrase, ValidationContext& context) {
    enterScope(context);
    analyzePhraseList(phrase->nested_phrases, context);
    leaveScope(context);
}

static void analyzeFunction(
    const std::shared_ptr<ParsedFunction>& function_phrase,
    ValidationContext& context
) {
    if (!function_phrase) {
        return;
    }

    const std::string return_type = normalizeTypeName(function_phrase->return_type.empty() ? "void" : function_phrase->return_type);
    if (context.symbols.known_types.find(return_type) == context.symbols.known_types.end()) {
        throw semanticError(function_phrase, "unknown function return type '" + function_phrase->return_type + "' in phrase: " + joinPhraseText(function_phrase));
    }

    const std::string previous_return_type = context.current_function_return_type;
    context.current_function_return_type = return_type;
    enterScope(context);

    analyzePhraseList(function_phrase->nested_phrases, context);

    leaveScope(context);
    context.current_function_return_type = previous_return_type;
}

static void analyzeVariableDeclaration(
    const std::shared_ptr<ParsedVariableDeclaration>& variable_phrase,
    ValidationContext& context
) {
    if (!variable_phrase) {
        return;
    }

    const std::string variable_type = normalizeTypeName(variable_phrase->type_name);
    if (!isKnownTypeRefString(context.symbols, variable_phrase->type_name)) {
        throw semanticError(variable_phrase, "unknown variable type '" + variable_phrase->type_name + "' in phrase: " + joinPhraseText(variable_phrase));
    }

    if (trim(variable_phrase->initializer).empty()) {
        throw semanticError(variable_phrase, "variable must have an initializer in phrase: " + joinPhraseText(variable_phrase));
    }

    SemanticExpressionIR initializer_expr = parseExpressionIR(variable_phrase->initializer, phraseStartLocation(variable_phrase));
    validateExpression(variable_phrase, initializer_expr, context);
    validateAssignmentCompatibility(variable_phrase, variable_type, initializer_expr, context);

    declareVariable(
        context,
        variable_phrase->name,
        variable_type,
        variable_phrase->is_mutable,
        variable_phrase
    );
}

static void analyzeTypeDefinition(
    const std::shared_ptr<ParsedTypeDefinition>& type_phrase,
    ValidationContext& context
) {
    if (!type_phrase) {
        return;
    }

    if (!isKnownTypeRefString(context.symbols, type_phrase->base_type)) {
        throw semanticError(type_phrase, "unknown base type '" + type_phrase->base_type + "' in phrase: " + joinPhraseText(type_phrase));
    }
}

static void analyzeStructDefinition(
    const std::shared_ptr<ParsedStructDefinition>& struct_phrase,
    ValidationContext& context
) {
    if (!struct_phrase) {
        return;
    }

    std::unordered_set<std::string> field_names{};
    for (const auto& nested : struct_phrase->nested_phrases) {
        if (!nested) {
            continue;
        }

        if (nested->kind != ParsedPhraseKind::ParameterDefinition) {
            analyzePhrase(nested, context);
            continue;
        }

        auto field = std::dynamic_pointer_cast<ParsedParameterDefinition>(nested);
        if (!field) {
            continue;
        }

        if (!isKnownTypeRefString(context.symbols, field->type_name)) {
            throw semanticError(field, "unknown struct field type '" + field->type_name + "' in phrase: " + joinPhraseText(field));
        }

        if (field_names.find(field->name) != field_names.end()) {
            throw semanticError(field, "duplicate struct field '" + field->name + "' in phrase: " + joinPhraseText(field));
        }

        field_names.insert(field->name);
    }
}

static void analyzeEnumDefinition(
    const std::shared_ptr<ParsedEnumDefinition>& enum_phrase,
    ValidationContext& context
) {
    (void)context;
    if (!enum_phrase) {
        return;
    }

    std::unordered_set<std::string> enum_values{};
    for (const auto& nested : enum_phrase->nested_phrases) {
        if (!nested || nested->kind != ParsedPhraseKind::EnumValueDefinition) {
            continue;
        }

        auto values = std::dynamic_pointer_cast<ParsedEnumValueDefinition>(nested);
        if (!values) {
            continue;
        }

        for (const auto& value : values->values) {
            const std::string trimmed_value = trim(value);
            if (trimmed_value.empty()) {
                continue;
            }

            if (enum_values.find(trimmed_value) != enum_values.end()) {
                throw semanticError(enum_phrase, "duplicate enum value '" + trimmed_value + "' in phrase: " + joinPhraseText(enum_phrase));
            }

            enum_values.insert(trimmed_value);
        }
    }
}

static void analyzeAssignment(
    const std::shared_ptr<ParsedAssignment>& assignment_phrase,
    ValidationContext& context
) {
    if (!assignment_phrase) {
        return;
    }

    const SemanticExpressionIR target_expr = parseExpressionIR(assignment_phrase->left, phraseStartLocation(assignment_phrase));
    if (!isAssignableExpression(target_expr)) {
        throw semanticError(assignment_phrase, "assignment target is not assignable in phrase: " + joinPhraseText(assignment_phrase));
    }

    const std::optional<std::string> root_name = expressionRootIdentifier(target_expr);
    if (!root_name.has_value()) {
        throw semanticError(assignment_phrase, "assignment target has no variable root in phrase: " + joinPhraseText(assignment_phrase));
    }

    VariableSymbol* symbol = lookupVariable(context, root_name.value());
    if (symbol == nullptr) {
        throw semanticError(assignment_phrase, "assignment to undeclared variable '" + root_name.value() + "' in phrase: " + joinPhraseText(assignment_phrase));
    }

    if (!symbol->is_mutable) {
        throw semanticError(assignment_phrase, "assignment to immutable variable '" + root_name.value() + "' in phrase: " + joinPhraseText(assignment_phrase));
    }

    SemanticExpressionIR expr = parseExpressionIR(assignment_phrase->right, phraseStartLocation(assignment_phrase));
    validateExpression(assignment_phrase, expr, context);
    validateAssignmentCompatibility(assignment_phrase, symbol->type_name, expr, context);
}

static void analyzeParameterDeclaration(
    const std::shared_ptr<ParsedParameterDeclaration>& parameter_phrase,
    ValidationContext& context
) {
    if (!parameter_phrase) {
        return;
    }

    const std::string parameter_type = normalizeTypeName(parameter_phrase->type_name);
    if (!isKnownTypeRefString(context.symbols, parameter_phrase->type_name)) {
        throw semanticError(parameter_phrase, "unknown parameter type '" + parameter_phrase->type_name + "' in phrase: " + joinPhraseText(parameter_phrase));
    }

    declareVariable(
        context,
        parameter_phrase->name,
        parameter_type,
        false,
        parameter_phrase
    );
}

static void analyzeCallStatement(
    const std::shared_ptr<ParsedCallStatement>& call_phrase,
    ValidationContext& context
) {
    if (!call_phrase) {
        return;
    }

    if (context.symbols.known_functions.find(call_phrase->name) == context.symbols.known_functions.end()) {
        throw semanticError(call_phrase, "unknown function '" + call_phrase->name + "'");
    }

    if (call_phrase->name != "print" && call_phrase->name != "println") {
        auto arity = context.symbols.function_parameter_counts.find(call_phrase->name);
        if (arity != context.symbols.function_parameter_counts.end() && arity->second != call_phrase->arguments.size()) {
            throw semanticError(call_phrase, "function '" + call_phrase->name + "' expects " + std::to_string(arity->second) + " argument(s), got " + std::to_string(call_phrase->arguments.size()));
        }
    }

    for (const auto& argument : call_phrase->arguments) {
        validateExpression(call_phrase, parseExpressionIR(argument, phraseStartLocation(call_phrase)), context);
    }
}

static void analyzeReturnStatement(
    const std::shared_ptr<ParsedReturnStatement>& return_phrase,
    ValidationContext& context
) {
    if (!return_phrase) {
        return;
    }

    const std::string expected_type = normalizeTypeName(context.current_function_return_type.empty() ? "void" : context.current_function_return_type);
    const std::string returned_expression = trim(return_phrase->expression);

    if (expected_type == "void") {
        if (!returned_expression.empty()) {
            throw semanticError(return_phrase, "void function cannot return a value");
        }
        return;
    }

    if (returned_expression.empty()) {
        throw semanticError(return_phrase, "non-void function must return a value");
    }

    SemanticExpressionIR expression = parseExpressionIR(returned_expression, phraseStartLocation(return_phrase));
    validateExpression(return_phrase, expression, context);
    validateAssignmentCompatibility(return_phrase, expected_type, expression, context);
}

static void analyzeUnknown(const std::shared_ptr<ParsedPhrase>& phrase) {
    throw semanticError(phrase, "unsupported phrase for analysis: " + phrase->label() + " -> " + joinPhraseText(phrase));
}

static void analyzePhrase(const std::shared_ptr<ParsedPhrase>& phrase, ValidationContext& context) {
    if (!phrase) {
        return;
    }

    switch (phrase->kind) {
        case ParsedPhraseKind::Function:
            analyzeFunction(std::dynamic_pointer_cast<ParsedFunction>(phrase), context);
            break;
        case ParsedPhraseKind::VariableDeclaration:
            analyzeVariableDeclaration(std::dynamic_pointer_cast<ParsedVariableDeclaration>(phrase), context);
            break;
        case ParsedPhraseKind::TypeDefinition:
            analyzeTypeDefinition(std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase), context);
            break;
        case ParsedPhraseKind::StructDefinition:
            analyzeStructDefinition(std::dynamic_pointer_cast<ParsedStructDefinition>(phrase), context);
            break;
        case ParsedPhraseKind::EnumDefinition:
            analyzeEnumDefinition(std::dynamic_pointer_cast<ParsedEnumDefinition>(phrase), context);
            break;
        case ParsedPhraseKind::Assignment:
            analyzeAssignment(std::dynamic_pointer_cast<ParsedAssignment>(phrase), context);
            break;
        case ParsedPhraseKind::ParameterDeclaration:
            analyzeParameterDeclaration(std::dynamic_pointer_cast<ParsedParameterDeclaration>(phrase), context);
            break;
        case ParsedPhraseKind::CallStatement:
            analyzeCallStatement(std::dynamic_pointer_cast<ParsedCallStatement>(phrase), context);
            break;
        case ParsedPhraseKind::ReturnStatement:
            analyzeReturnStatement(std::dynamic_pointer_cast<ParsedReturnStatement>(phrase), context);
            break;
        case ParsedPhraseKind::BreakStatement:
            if (context.loop_depth == 0) {
                throw semanticError(phrase, "break used outside of a loop");
            }
            break;
        case ParsedPhraseKind::ContinueStatement:
            if (context.loop_depth == 0) {
                throw semanticError(phrase, "continue used outside of a loop");
            }
            break;
        case ParsedPhraseKind::IfStatement:
        case ParsedPhraseKind::ElseStatement:
            analyzeControlFlowBody(phrase, context);
            break;
        case ParsedPhraseKind::WhileStatement:
        case ParsedPhraseKind::ForStatement:
            ++context.loop_depth;
            analyzeControlFlowBody(phrase, context);
            --context.loop_depth;
            break;
        case ParsedPhraseKind::Unknown:
            analyzeUnknown(phrase);
            break;
        case ParsedPhraseKind::ParameterDefinition:
        case ParsedPhraseKind::EnumValueDefinition:
            break;
        default:
            throw semanticError(phrase, "unsupported parsed phrase kind");
    }
}

static void analyzePhraseList(const std::vector<std::shared_ptr<ParsedPhrase>>& phrases, ValidationContext& context) {
    bool previous_allows_else = false;

    for (const auto& phrase : phrases) {
        if (!phrase) {
            continue;
        }

        if (phrase->kind == ParsedPhraseKind::ElseStatement && !previous_allows_else) {
            throw semanticError(phrase, "else without a preceding if or else-if");
        }

        analyzePhrase(phrase, context);

        previous_allows_else = phrase->kind == ParsedPhraseKind::IfStatement;
        if (phrase->kind == ParsedPhraseKind::ElseStatement) {
            auto else_phrase = std::dynamic_pointer_cast<ParsedElseStatement>(phrase);
            previous_allows_else = else_phrase && !else_phrase->condition.empty();
        }
    }
}

static SemanticSymbolTable collectSymbols(const ParsedPhrases& phrases) {
    SemanticSymbolTable symbols{};

    const std::vector<std::string> primitive_types = {
        "bool", "char8", "char16", "char32", "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64", "float32", "float64", "void"
    };

    for (const auto& primitive : primitive_types) {
        symbols.known_types.insert({primitive, TypeSymbol{TypeSymbolKind::Primitive, ""}});
    }

    symbols.known_functions.insert("print");
    symbols.known_functions.insert("println");
    symbols.function_return_types["print"] = "void";
    symbols.function_return_types["println"] = "void";
    symbols.function_parameter_counts["print"] = 0;
    symbols.function_parameter_counts["println"] = 0;

    for (const auto& phrase : phrases) {
        collectSymbolsFromPhrase(phrase, symbols);
    }

    return symbols;
}

static void validateSemantics(const ParsedPhrases& phrases, const SemanticSymbolTable& symbols) {
    ValidationContext validation_context{symbols};
    enterScope(validation_context);

    analyzePhraseList(phrases, validation_context);

    leaveScope(validation_context);
}

static SemanticVariableDeclarationIR lowerVariableDeclaration(const std::shared_ptr<ParsedVariableDeclaration>& phrase) {
    SemanticVariableDeclarationIR node{};
    node.name = phrase->name;
    node.type = parseTypeRef(phrase->type_name);
    node.initializer = parseExpressionIR(phrase->initializer, phraseStartLocation(phrase));
    node.is_mutable = phrase->is_mutable;
    node.location = phraseStartLocation(phrase);
    return node;
}

static SemanticAssignmentIR lowerAssignment(const std::shared_ptr<ParsedAssignment>& phrase) {
    SemanticAssignmentIR node{};
    node.target = trim(phrase->left);
    node.operator_symbol = phrase->operator_symbol;
    node.expression = parseExpressionIR(phrase->right, phraseStartLocation(phrase));
    node.location = phraseStartLocation(phrase);
    return node;
}

static SemanticTypeDefinitionIR lowerTypeDefinition(const std::shared_ptr<ParsedTypeDefinition>& phrase) {
    SemanticTypeDefinitionIR node{};
    node.name = phrase->name;
    node.base_type = parseTypeRef(phrase->base_type);
    node.location = phraseStartLocation(phrase);
    return node;
}

static SemanticStructDefinitionIR lowerStructDefinition(const std::shared_ptr<ParsedStructDefinition>& phrase) {
    SemanticStructDefinitionIR node{};
    node.name = phrase->name;
    node.location = phraseStartLocation(phrase);

    for (const auto& nested : phrase->nested_phrases) {
        if (!nested || nested->kind != ParsedPhraseKind::ParameterDefinition) {
            continue;
        }

        auto field_phrase = std::dynamic_pointer_cast<ParsedParameterDefinition>(nested);
        if (!field_phrase) {
            continue;
        }

        SemanticStructFieldIR field{};
        field.name = field_phrase->name;
        field.type = parseTypeRef(field_phrase->type_name);
        field.location = phraseStartLocation(field_phrase);
        node.fields.push_back(field);
    }

    return node;
}

static SemanticEnumDefinitionIR lowerEnumDefinition(const std::shared_ptr<ParsedEnumDefinition>& phrase) {
    SemanticEnumDefinitionIR node{};
    node.name = phrase->name;
    node.location = phraseStartLocation(phrase);

    for (const auto& nested : phrase->nested_phrases) {
        if (!nested || nested->kind != ParsedPhraseKind::EnumValueDefinition) {
            continue;
        }

        auto values_phrase = std::dynamic_pointer_cast<ParsedEnumValueDefinition>(nested);
        if (!values_phrase) {
            continue;
        }

        for (const auto& value : values_phrase->values) {
            const std::string trimmed_value = trim(value);
            if (!trimmed_value.empty()) {
                node.values.push_back(trimmed_value);
            }
        }
    }

    return node;
}

static void lowerBodyPhrases(
    const std::vector<std::shared_ptr<ParsedPhrase>>& phrases,
    std::vector<SemanticVariableDeclarationIR>& variable_declarations,
    std::vector<SemanticAssignmentIR>& assignments,
    std::vector<SemanticTypeDefinitionIR>& type_definitions,
    std::vector<SemanticStructDefinitionIR>& struct_definitions,
    std::vector<SemanticEnumDefinitionIR>& enum_definitions,
    std::vector<SemanticCallIR>& calls,
    std::vector<SemanticReturnIR>& returns,
    std::vector<SemanticIfIR>& if_statements,
    std::vector<SemanticWhileIR>& while_statements,
    std::vector<SemanticForIR>& for_statements,
    std::vector<SemanticElseIR>& else_statements,
    std::vector<SemanticStatementRef>& order
);

static void lowerBodyPhrases(
    const std::vector<std::shared_ptr<ParsedPhrase>>& phrases,
    std::vector<SemanticVariableDeclarationIR>& variable_declarations,
    std::vector<SemanticAssignmentIR>& assignments,
    std::vector<SemanticTypeDefinitionIR>& type_definitions,
    std::vector<SemanticStructDefinitionIR>& struct_definitions,
    std::vector<SemanticEnumDefinitionIR>& enum_definitions,
    std::vector<SemanticCallIR>& calls,
    std::vector<SemanticReturnIR>& returns,
    std::vector<SemanticIfIR>& if_statements,
    std::vector<SemanticWhileIR>& while_statements,
    std::vector<SemanticForIR>& for_statements,
    std::vector<SemanticElseIR>& else_statements,
    std::vector<SemanticStatementRef>& order
) {
    for (const auto& phrase : phrases) {
        if (!phrase) {
            continue;
        }

        switch (phrase->kind) {
            case ParsedPhraseKind::VariableDeclaration: {
                auto node = lowerVariableDeclaration(std::dynamic_pointer_cast<ParsedVariableDeclaration>(phrase));
                variable_declarations.push_back(node);
                order.push_back({SemanticStatementKind::VariableDeclaration, variable_declarations.size() - 1});
                break;
            }
            case ParsedPhraseKind::Assignment: {
                auto node = lowerAssignment(std::dynamic_pointer_cast<ParsedAssignment>(phrase));
                assignments.push_back(node);
                order.push_back({SemanticStatementKind::Assignment, assignments.size() - 1});
                break;
            }
            case ParsedPhraseKind::TypeDefinition: {
                auto node = lowerTypeDefinition(std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase));
                type_definitions.push_back(node);
                order.push_back({SemanticStatementKind::TypeDefinition, type_definitions.size() - 1});
                break;
            }
            case ParsedPhraseKind::StructDefinition: {
                auto node = lowerStructDefinition(std::dynamic_pointer_cast<ParsedStructDefinition>(phrase));
                struct_definitions.push_back(node);
                order.push_back({SemanticStatementKind::StructDefinition, struct_definitions.size() - 1});
                break;
            }
            case ParsedPhraseKind::EnumDefinition: {
                auto node = lowerEnumDefinition(std::dynamic_pointer_cast<ParsedEnumDefinition>(phrase));
                enum_definitions.push_back(node);
                order.push_back({SemanticStatementKind::EnumDefinition, enum_definitions.size() - 1});
                break;
            }
            case ParsedPhraseKind::CallStatement: {
                auto call_phrase = std::dynamic_pointer_cast<ParsedCallStatement>(phrase);
                if (!call_phrase) {
                    break;
                }

                SemanticCallIR node{};
                node.name = call_phrase->name;
                node.location = phraseStartLocation(call_phrase);
                for (const auto& arg : call_phrase->arguments) {
                    node.arguments.push_back(parseExpressionIR(arg, node.location));
                }
                calls.push_back(node);
                order.push_back({SemanticStatementKind::Call, calls.size() - 1});
                break;
            }
            case ParsedPhraseKind::ReturnStatement: {
                auto return_phrase = std::dynamic_pointer_cast<ParsedReturnStatement>(phrase);
                if (!return_phrase) {
                    break;
                }

                SemanticReturnIR node{};
                node.expression = parseExpressionIR(return_phrase->expression, phraseStartLocation(return_phrase));
                node.location = phraseStartLocation(return_phrase);
                returns.push_back(node);
                order.push_back({SemanticStatementKind::Return, returns.size() - 1});
                break;
            }
            case ParsedPhraseKind::BreakStatement:
                order.push_back({SemanticStatementKind::Break, 0});
                break;
            case ParsedPhraseKind::ContinueStatement:
                order.push_back({SemanticStatementKind::Continue, 0});
                break;
            case ParsedPhraseKind::IfStatement: {
                    auto if_phrase = std::dynamic_pointer_cast<ParsedIfStatement>(phrase);
                    SemanticIfIR node{};
                    node.condition = parseExpressionIR(if_phrase ? if_phrase->condition : "", phraseStartLocation(phrase));
                    node.location = phraseStartLocation(phrase);
                    lowerBodyPhrases(
                        phrase->nested_phrases,
                        variable_declarations,
                        assignments,
                        type_definitions,
                        struct_definitions,
                        enum_definitions,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body
                    );
                    if_statements.push_back(node);
                    order.push_back({SemanticStatementKind::If, if_statements.size() - 1});
                break;
            }
            case ParsedPhraseKind::WhileStatement: {
                    auto while_phrase = std::dynamic_pointer_cast<ParsedWhileStatement>(phrase);
                    SemanticWhileIR node{};
                    node.condition = parseExpressionIR(while_phrase ? while_phrase->condition : "", phraseStartLocation(phrase));
                    node.location = phraseStartLocation(phrase);
                    lowerBodyPhrases(
                        phrase->nested_phrases,
                        variable_declarations,
                        assignments,
                        type_definitions,
                        struct_definitions,
                        enum_definitions,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body
                    );
                    while_statements.push_back(node);
                    order.push_back({SemanticStatementKind::While, while_statements.size() - 1});
                break;
            }
            case ParsedPhraseKind::ForStatement: {
                    auto for_phrase = std::dynamic_pointer_cast<ParsedForStatement>(phrase);
                    SemanticForIR node{};
                    node.initializer = parseExpressionIR(for_phrase ? for_phrase->initializer : "", phraseStartLocation(phrase));
                    node.condition = parseExpressionIR(for_phrase ? for_phrase->condition : "", phraseStartLocation(phrase));
                    node.update = parseExpressionIR(for_phrase ? for_phrase->update : "", phraseStartLocation(phrase));
                    node.location = phraseStartLocation(phrase);
                    lowerBodyPhrases(
                        phrase->nested_phrases,
                        variable_declarations,
                        assignments,
                        type_definitions,
                        struct_definitions,
                        enum_definitions,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body
                    );
                    for_statements.push_back(node);
                    order.push_back({SemanticStatementKind::For, for_statements.size() - 1});
                break;
            }
            case ParsedPhraseKind::ElseStatement: {
                    auto else_phrase = std::dynamic_pointer_cast<ParsedElseStatement>(phrase);
                    SemanticElseIR node{};
                    node.condition = parseExpressionIR(else_phrase ? else_phrase->condition : "", phraseStartLocation(phrase));
                    node.location = phraseStartLocation(phrase);
                    lowerBodyPhrases(
                        phrase->nested_phrases,
                        variable_declarations,
                        assignments,
                        type_definitions,
                        struct_definitions,
                        enum_definitions,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body
                    );
                    else_statements.push_back(node);
                    order.push_back({SemanticStatementKind::Else, else_statements.size() - 1});
                break;
            }
            default:
                break;
        }
    }
}

static SemanticFunctionIR lowerFunction(const std::shared_ptr<ParsedFunction>& phrase) {
    SemanticFunctionIR function_ir{};
    function_ir.name = phrase->name;
    function_ir.return_type = phrase->return_type.empty() ? parseTypeRef("void") : parseTypeRef(phrase->return_type);
    function_ir.location = phraseStartLocation(phrase);

    for (const auto& nested : phrase->nested_phrases) {
        if (!nested) {
            continue;
        }

        if (nested->kind == ParsedPhraseKind::ParameterDeclaration) {
            auto param_phrase = std::dynamic_pointer_cast<ParsedParameterDeclaration>(nested);
            if (!param_phrase) {
                continue;
            }

            SemanticParameterIR parameter{};
            parameter.name = param_phrase->name;
            parameter.type = parseTypeRef(param_phrase->type_name);
            parameter.location = phraseStartLocation(param_phrase);
            function_ir.parameters.push_back(parameter);
        }
    }

    lowerBodyPhrases(
        phrase->nested_phrases,
        function_ir.variable_declarations,
        function_ir.assignments,
        function_ir.type_definitions,
        function_ir.struct_definitions,
        function_ir.enum_definitions,
        function_ir.calls,
        function_ir.returns,
        function_ir.if_statements,
        function_ir.while_statements,
        function_ir.for_statements,
        function_ir.else_statements,
        function_ir.body_order
    );

    return function_ir;
}

static void lowerTopLevelPhrase(const std::shared_ptr<ParsedPhrase>& phrase, SemanticProgram& program) {
    if (!phrase) {
        return;
    }

    switch (phrase->kind) {
        case ParsedPhraseKind::Function:
            program.functions.push_back(lowerFunction(std::dynamic_pointer_cast<ParsedFunction>(phrase)));
            break;
        case ParsedPhraseKind::VariableDeclaration: {
            program.global_variables.push_back(lowerVariableDeclaration(std::dynamic_pointer_cast<ParsedVariableDeclaration>(phrase)));
            program.top_level_order.push_back({SemanticStatementKind::VariableDeclaration, program.global_variables.size() - 1});
            break;
        }
        case ParsedPhraseKind::Assignment: {
            program.global_assignments.push_back(lowerAssignment(std::dynamic_pointer_cast<ParsedAssignment>(phrase)));
            program.top_level_order.push_back({SemanticStatementKind::Assignment, program.global_assignments.size() - 1});
            break;
        }
        case ParsedPhraseKind::TypeDefinition: {
            program.type_definitions.push_back(lowerTypeDefinition(std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase)));
            program.top_level_order.push_back({SemanticStatementKind::TypeDefinition, program.type_definitions.size() - 1});
            break;
        }
        case ParsedPhraseKind::StructDefinition: {
            program.struct_definitions.push_back(lowerStructDefinition(std::dynamic_pointer_cast<ParsedStructDefinition>(phrase)));
            program.top_level_order.push_back({SemanticStatementKind::StructDefinition, program.struct_definitions.size() - 1});
            break;
        }
        case ParsedPhraseKind::EnumDefinition: {
            program.enum_definitions.push_back(lowerEnumDefinition(std::dynamic_pointer_cast<ParsedEnumDefinition>(phrase)));
            program.top_level_order.push_back({SemanticStatementKind::EnumDefinition, program.enum_definitions.size() - 1});
            break;
        }
        default:
            break;
    }
}

SemanticProgram buildSemanticProgram(const ParsedPhrases& phrases) {
    const SemanticSymbolTable symbols = collectSymbols(phrases);
    validateSemantics(phrases, symbols);

    SemanticProgram program{};
    for (const auto& phrase : phrases) {
        lowerTopLevelPhrase(phrase, program);
    }

    return program;
}

void analyzeParsedPhrases(const ParsedPhrases& phrases) {
    const SemanticSymbolTable symbols = collectSymbols(phrases);
    validateSemantics(phrases, symbols);
}
