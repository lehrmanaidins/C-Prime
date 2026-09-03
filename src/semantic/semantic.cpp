#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <optional>
#include <regex>
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
#include "../parser/phrases/import.cpp"
#include "../parser/phrases/parameter_declaration.cpp"
#include "../parser/phrases/parameter_definition.cpp"
#include "../parser/phrases/return_statement.cpp"
#include "../parser/phrases/statement.cpp"
#include "../parser/phrases/struct_definition.cpp"
#include "../parser/phrases/type_definition.cpp"
#include "../parser/phrases/union_definition.cpp"
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

struct SemanticFunctionIR;

struct SemanticGenericParameterIR {
    std::string name;
};

struct SemanticTypeDefinitionIR {
    std::string name;
    std::vector<SemanticGenericParameterIR> template_parameters;
    SemanticTypeRef base_type;
    SourceLocation location;
    bool has_requires = false;
    SemanticExpressionIR requires_clause;
    bool has_ensures = false;
    SemanticExpressionIR ensures_clause;
    std::vector<SemanticFunctionIR> member_functions;
};

struct SemanticStructFieldIR {
    std::string name;
    SemanticTypeRef type;
    SourceLocation location;
};

struct SemanticStructDefinitionIR {
    std::string name;
    std::vector<SemanticGenericParameterIR> template_parameters;
    std::vector<SemanticStructFieldIR> fields;
    SourceLocation location;
};

struct SemanticEnumDefinitionIR {
    std::string name;
    std::vector<std::string> values;
    std::string underlying_cpp_type;
    SourceLocation location;
};

enum class SemanticImportKind {
    Cpp
};

struct SemanticImportIR {
    std::string path;
    SemanticImportKind import_kind;
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
    Import,
    If,
    While,
    For,
    Else,
    Call,
    Return,
    Break,
    Continue,
    Trivia,
    Function
};

struct SemanticTriviaIR {
    std::vector<std::string> lines;
    std::string suffix;
};

static std::vector<std::string> triviaToLines(const std::vector<PhraseTrivia>& trivia) {
    std::vector<std::string> lines;
    for (const auto& item : trivia) {
        lines.push_back(item.kind == PhraseTriviaKind::Comment ? item.text : std::string{});
    }
    return lines;
}

static SemanticTriviaIR makeLeadingTrivia(const std::vector<PhraseTrivia>& trivia) {
    SemanticTriviaIR node{};
    node.lines = triviaToLines(trivia);
    return node;
}

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

struct SemanticTemplateParameterIR {
    std::string name;
    std::vector<SemanticTypeRef> allowed_types;
    std::string category;
};

struct SemanticFunctionIR {
    std::string name;
    SemanticTypeRef return_type;
    std::string return_value_name;
    bool has_requires = false;
    SemanticExpressionIR requires_clause;
    bool has_ensures = false;
    SemanticExpressionIR ensures_clause;
    std::vector<SemanticTemplateParameterIR> template_parameters;
    std::vector<SemanticParameterIR> parameters;
    std::vector<std::string> tags;
    bool is_discardable = false;

    std::vector<SemanticVariableDeclarationIR> variable_declarations;
    std::vector<SemanticAssignmentIR> assignments;
    std::vector<SemanticTypeDefinitionIR> type_definitions;
    std::vector<SemanticStructDefinitionIR> struct_definitions;
    std::vector<SemanticEnumDefinitionIR> enum_definitions;
    std::vector<SemanticImportIR> imports;
    std::vector<SemanticCallIR> calls;
    std::vector<SemanticReturnIR> returns;
    std::vector<SemanticIfIR> if_statements;
    std::vector<SemanticWhileIR> while_statements;
    std::vector<SemanticForIR> for_statements;
    std::vector<SemanticElseIR> else_statements;
    std::vector<SemanticTriviaIR> trivia;
    std::vector<std::string> leading_trivia_lines;
    std::vector<std::string> trailing_trivia_lines;
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
    std::vector<SemanticImportIR> imports;
    std::vector<SemanticCallIR> calls;
    std::vector<SemanticIfIR> if_statements;
    std::vector<SemanticWhileIR> while_statements;
    std::vector<SemanticForIR> for_statements;
    std::vector<SemanticElseIR> else_statements;
    std::vector<SemanticTriviaIR> trivia;
    std::vector<std::string> trailing_trivia_lines;
    std::vector<SemanticStatementRef> top_level_order;
    std::unordered_map<std::string, std::vector<std::string>> union_members;
};

enum class TypeSymbolKind {
    Primitive,
    Domain,
    Struct,
    Enum,
    Union
};

struct TypeSymbol {
    TypeSymbolKind kind;
    std::string underlying;
    size_t template_parameter_count = 0;
};

struct VariableSymbol {
    std::string type_name;
    bool is_mutable;
    bool is_parameter = false;
};

struct TemplateTypeConstraint {
    std::vector<std::string> allowed_types;
    std::string category;
};

struct SemanticSymbolTable {
    std::unordered_map<std::string, TypeSymbol> known_types;
    std::unordered_set<std::string> known_functions;
    std::unordered_map<std::string, std::string> function_return_types;
    std::unordered_map<std::string, size_t> function_parameter_counts;
    std::unordered_map<std::string, std::vector<std::string>> function_parameter_types;
    std::unordered_map<std::string, std::unordered_map<std::string, TemplateTypeConstraint>> function_template_constraints;
    std::unordered_map<std::string, std::vector<std::string>> union_members;
    std::unordered_set<std::string> variadic_functions;
    std::unordered_map<std::string, VariableSymbol> external_values;
    std::unordered_map<std::string, std::unordered_map<std::string, size_t>> type_member_functions;
    bool has_cpp_imports = false;
};

struct ValidationContext {
    const SemanticSymbolTable& symbols;
    std::vector<std::unordered_map<std::string, VariableSymbol>> variable_scopes;
    std::unordered_set<std::string> template_type_parameters;
    std::string current_function_return_type;
    size_t loop_depth = 0;
    size_t unsafe_depth = 0;
    bool current_function_is_unsafe = false;
};

#include "cpp_import_symbols.cpp"

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

// For a generic instantiation such as `Box<DomainType>`, returns the bare type
// name (`Box`) so it can be looked up in the symbol table. `reference<...>`,
// `pointer<...>` and `function<...>` keep their full spelling because they are
// handled as dedicated type kinds elsewhere.
static std::string stripGenericArguments(const std::string& type_name) {
    const size_t open = type_name.find('<');
    if (open == std::string::npos) {
        return type_name;
    }

    const std::string head = trim(type_name.substr(0, open));
    if (head.empty() || head == "reference" || head == "pointer" || head == "function") {
        return type_name;
    }

    for (char ch : head) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            return type_name;
        }
    }

    return head;
}

static bool isKnownType(const SemanticSymbolTable& symbols, const std::string& raw_type) {
    const std::string type = normalizeTypeName(raw_type);
    if (type.empty()) {
        return false;
    }

    return symbols.known_types.find(type) != symbols.known_types.end();
}

static bool isKnownTypeRef(
    const SemanticSymbolTable& symbols,
    const SemanticTypeRef& type_ref,
    const std::unordered_set<std::string>& template_type_parameters = {}
) {
    if (type_ref.kind == SemanticTypeKind::Tuple) {
        for (const auto& element : type_ref.tuple_elements) {
            if (!isKnownTypeRef(symbols, element, template_type_parameters)) {
                return false;
            }
        }
        return true;
    }

    if (type_ref.kind == SemanticTypeKind::Reference) {
        return type_ref.reference_target && isKnownTypeRef(symbols, *type_ref.reference_target, template_type_parameters);
    }

    if (type_ref.kind == SemanticTypeKind::Pointer) {
        return type_ref.pointer_target && isKnownTypeRef(symbols, *type_ref.pointer_target, template_type_parameters);
    }

    if (type_ref.kind == SemanticTypeKind::Function) {
        if (!type_ref.function_return_type || !isKnownTypeRef(symbols, *type_ref.function_return_type, template_type_parameters)) {
            return false;
        }
        for (const auto& parameter : type_ref.function_parameters) {
            if (!isKnownTypeRef(symbols, parameter, template_type_parameters)) {
                return false;
            }
        }
        return true;
    }

    if (type_ref.name.empty()) {
        return false;
    }

    const std::string name = normalizeTypeName(type_ref.name);

    if (!type_ref.generic_arguments.empty()) {
        const auto generic_it = symbols.known_types.find(name);
        if (generic_it == symbols.known_types.end()
            || generic_it->second.template_parameter_count != type_ref.generic_arguments.size()) {
            return false;
        }
        for (const auto& argument : type_ref.generic_arguments) {
            if (!isKnownTypeRef(symbols, argument, template_type_parameters)) {
                return false;
            }
        }
        return true;
    }

    return symbols.known_types.find(name) != symbols.known_types.end()
        || template_type_parameters.find(name) != template_type_parameters.end();
}

static bool isKnownTypeRefString(
    const SemanticSymbolTable& symbols,
    const std::string& raw_type,
    const std::unordered_set<std::string>& template_type_parameters = {}
) {
    return isKnownTypeRef(symbols, parseTypeRef(raw_type), template_type_parameters);
}

static bool isTypeUnion(const SemanticSymbolTable& symbols, const std::string& raw_type) {
    return symbols.union_members.find(normalizeTypeName(raw_type)) != symbols.union_members.end();
}

static bool containsPointerType(const SemanticTypeRef& type_ref) {
    if (type_ref.kind == SemanticTypeKind::Pointer) {
        return true;
    }

    if (type_ref.kind == SemanticTypeKind::Reference) {
        return type_ref.reference_target && containsPointerType(*type_ref.reference_target);
    }

    if (type_ref.kind == SemanticTypeKind::Tuple) {
        for (const auto& element : type_ref.tuple_elements) {
            if (containsPointerType(element)) {
                return true;
            }
        }
    }

    return false;
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
    const std::shared_ptr<ParsedPhrase>& phrase,
    bool is_parameter = false
) {
    if (context.variable_scopes.empty()) {
        enterScope(context);
    }

    auto& scope = context.variable_scopes.back();
    if (scope.find(name) != scope.end()) {
        throw semanticError(phrase, "duplicate declaration of variable '" + name + "' in phrase: " + joinPhraseText(phrase));
    }

    scope.insert({name, VariableSymbol{type_name, is_mutable, is_parameter}});
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
    for (const auto& parameter : function_phrase->parameters) {
        auto parameter_declaration = std::dynamic_pointer_cast<ParsedParameterDeclaration>(parameter);
        if (parameter_declaration) {
            symbols.function_parameter_types[function_phrase->name].push_back(normalizeTypeName(parameter_declaration->type_name));
        }
    }
    for (const auto& parameter : function_phrase->template_parameters) {
        symbols.function_template_constraints[function_phrase->name].insert({
            parameter.name,
            TemplateTypeConstraint{parameter.allowed_types, parameter.category}
        });
    }

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
                    TypeSymbol{TypeSymbolKind::Domain, trim(type_def->base_type), type_def->template_parameters.size()},
                    symbols
                );

                for (const auto& nested : phrase->nested_phrases) {
                    if (!nested || nested->kind != ParsedPhraseKind::Function) {
                        continue;
                    }
                    auto member_function = std::dynamic_pointer_cast<ParsedFunction>(nested);
                    if (!member_function) {
                        continue;
                    }
                    symbols.type_member_functions[type_def->name][member_function->name] = member_function->parameters.size();
                }
            }
            break;
        }
        case ParsedPhraseKind::StructDefinition: {
            auto struct_def = std::dynamic_pointer_cast<ParsedStructDefinition>(phrase);
            if (struct_def) {
                registerTypeSymbol(
                    phrase,
                    struct_def->name,
                    TypeSymbol{TypeSymbolKind::Struct, "", struct_def->template_parameters.size()},
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
        case ParsedPhraseKind::UnionDefinition: {
            auto union_def = std::dynamic_pointer_cast<ParsedUnionDefinition>(phrase);
            if (union_def) {
                registerTypeSymbol(phrase, union_def->name, TypeSymbol{TypeSymbolKind::Union, ""}, symbols);
                symbols.union_members.insert({union_def->name, union_def->members});
            }
            break;
        }
        case ParsedPhraseKind::ImportStatement: {
            auto import_statement = std::dynamic_pointer_cast<ParsedImportStatement>(phrase);
            if (import_statement && import_statement->import_kind == ParsedImportKind::Cpp) {
                symbols.has_cpp_imports = true;
                registerCppImportSymbols(import_statement->path, symbols);
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

static void validateTypeUnionMembers(const SemanticSymbolTable& symbols) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> visiting;

    auto validate_union = [&](auto&& self, const std::string& name) -> void {
        if (visited.find(name) != visited.end()) {
            return;
        }
        if (!visiting.insert(name).second) {
            throw std::runtime_error("Semantic error: cyclic union definition involving '" + name + "'");
        }

        const auto members_it = symbols.union_members.find(name);
        if (members_it == symbols.union_members.end() || members_it->second.empty()) {
            throw std::runtime_error("Semantic error: union '" + name + "' must declare at least one member type");
        }
        for (const auto& member : members_it->second) {
            const std::string normalized_member = normalizeTypeName(member);
            if (symbols.known_types.find(normalized_member) == symbols.known_types.end()) {
                throw std::runtime_error("Semantic error: unknown union member type '" + member + "' in union '" + name + "'");
            }
            if (symbols.union_members.find(normalized_member) != symbols.union_members.end()) {
                self(self, normalized_member);
            }
        }

        visiting.erase(name);
        visited.insert(name);
    };

    for (const auto& [name, members] : symbols.union_members) {
        (void)members;
        validate_union(validate_union, name);
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
            auto external_it = context.symbols.external_values.find(trim(expression.text));
            if (external_it != context.symbols.external_values.end()) {
                return std::optional<std::string>{external_it->second.type_name};
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
            if ((expression.operator_symbol == "pointer" || expression.operator_symbol == "reference")
                && expression.children.size() == 1) {
                const std::optional<std::string> argument_type = inferExpressionTypeName(expression.children.front(), context);
                return argument_type.has_value()
                    ? std::optional<std::string>{expression.operator_symbol + "<" + argument_type.value() + ">"}
                    : std::nullopt;
            }
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
        || type_name == "float16" || type_name == "float32" || type_name == "float64" || type_name == "float128";
}

static bool templateConstraintRequiresPrimitivePrefix(const std::string& type_name, const SemanticSymbolTable& symbols) {
    const std::string normalized_type = normalizeTypeName(type_name);
    const auto type_it = symbols.known_types.find(normalized_type);
    return type_it != symbols.known_types.end() && type_it->second.kind == TypeSymbolKind::Primitive;
}

static bool isTemplateConstraintMatch(
    const std::string& type_name,
    const TemplateTypeConstraint& constraint,
    const SemanticSymbolTable& symbols,
    std::unordered_set<std::string>& resolving_unions
) {
    const std::string normalized_type = normalizeTypeName(type_name);
    if (constraint.category == "Integral") {
        return normalized_type == "int8" || normalized_type == "int16" || normalized_type == "int32" || normalized_type == "int64"
            || normalized_type == "uint8" || normalized_type == "uint16" || normalized_type == "uint32" || normalized_type == "uint64";
    }
    if (constraint.category == "Floating") {
        return normalized_type == "float32" || normalized_type == "float64";
    }

    return std::find_if(constraint.allowed_types.begin(), constraint.allowed_types.end(), [&](const std::string& allowed_type) {
        const std::string normalized_allowed = normalizeTypeName(allowed_type);
        if (normalized_allowed == normalized_type) {
            return true;
        }
        const auto members_it = symbols.union_members.find(normalized_allowed);
        if (members_it == symbols.union_members.end() || !resolving_unions.insert(normalized_allowed).second) {
            return false;
        }
        const TemplateTypeConstraint nested_constraint{members_it->second, ""};
        const bool matches = isTemplateConstraintMatch(normalized_type, nested_constraint, symbols, resolving_unions);
        resolving_unions.erase(normalized_allowed);
        return matches;
    }) != constraint.allowed_types.end();
}

static void validateTemplateCallConstraints(
    const std::shared_ptr<ParsedPhrase>& phrase,
    const std::string& function_name,
    const std::vector<SemanticExpressionIR>& arguments,
    ValidationContext& context
) {
    const auto constraints_it = context.symbols.function_template_constraints.find(function_name);
    const auto parameter_types_it = context.symbols.function_parameter_types.find(function_name);
    if (constraints_it == context.symbols.function_template_constraints.end()
        || parameter_types_it == context.symbols.function_parameter_types.end()) {
        return;
    }

    const auto& constraints = constraints_it->second;
    const auto& parameter_types = parameter_types_it->second;
    for (size_t i = 0; i < arguments.size() && i < parameter_types.size(); ++i) {
        const auto constraint_it = constraints.find(parameter_types[i]);
        if (constraint_it == constraints.end()) {
            continue;
        }

        if (constraint_it->second.allowed_types.empty() && constraint_it->second.category.empty()) {
            continue;
        }

        const std::optional<std::string> argument_type = inferExpressionTypeName(arguments[i], context);
        std::unordered_set<std::string> resolving_unions;
        if (argument_type.has_value() && !isTemplateConstraintMatch(argument_type.value(), constraint_it->second, context.symbols, resolving_unions)) {
            throw semanticError(
                phrase,
                "template parameter '" + parameter_types[i] + "' does not allow argument type '" + argument_type.value() + "'"
            );
        }
    }
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
    if (expression.kind == SemanticExpressionKind::Identifier) {
        const std::string name = trim(expression.text);
        if (lookupVariable(context, name) == nullptr
            && context.symbols.external_values.find(name) == context.symbols.external_values.end()) {
            throw semanticError(phrase, "unknown value '" + name + "'");
        }
        return;
    }

    if (expression.kind == SemanticExpressionKind::QualifiedName) {
        if (expression.children.size() == 2) {
            const std::string qualifier = trim(expression.children[0].text);
            if (context.symbols.known_types.find(normalizeTypeName(qualifier)) == context.symbols.known_types.end()) {
                throw semanticError(phrase, "unknown type '" + qualifier + "'");
            }
        }
        return;
    }

    if (expression.kind == SemanticExpressionKind::MemberAccess) {
        if (!expression.children.empty()) {
            validateExpression(phrase, expression.children.front(), context);
        }
        return;
    }

    if (expression.kind == SemanticExpressionKind::Call) {
        if (expression.operator_symbol == "pointer") {
            if (context.unsafe_depth == 0) {
                throw semanticError(phrase, "'pointer()' can only be called inside an unsafe block or unsafe function");
            }
            if (expression.children.size() != 1 || !isAssignableExpression(expression.children.front())) {
                throw semanticError(phrase, "'pointer()' expects a single variable argument");
            }
        } else if (expression.operator_symbol == "reference") {
            if (expression.children.size() != 1 || !isAssignableExpression(expression.children.front())) {
                throw semanticError(phrase, "'reference()' expects a single variable argument");
            }
        } else if (context.symbols.known_functions.find(expression.operator_symbol) == context.symbols.known_functions.end()) {
            throw semanticError(phrase, "unknown function '" + expression.operator_symbol + "'");
        } else if (context.symbols.variadic_functions.find(expression.operator_symbol) == context.symbols.variadic_functions.end()) {
            auto arity = context.symbols.function_parameter_counts.find(expression.operator_symbol);
            if (arity != context.symbols.function_parameter_counts.end() && arity->second != expression.children.size()) {
                throw semanticError(phrase, "function '" + expression.operator_symbol + "' expects " + std::to_string(arity->second) + " argument(s), got " + std::to_string(expression.children.size()));
            }
        }
        validateTemplateCallConstraints(phrase, expression.operator_symbol, expression.children, context);
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
    const std::string normalized_target = stripGenericArguments(normalizeTypeName(target_type));
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

        const std::string source_type = stripGenericArguments(normalizeTypeName(expression_type.value()));
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

        const std::string source_type = stripGenericArguments(normalizeTypeName(expression_type.value()));
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

static void analyzeForStatement(const std::shared_ptr<ParsedForStatement>& for_phrase, ValidationContext& context) {
    if (!for_phrase) {
        return;
    }

    enterScope(context);
    if (!for_phrase->initializer.empty()) {
        const size_t equal_pos = for_phrase->initializer.find('=');
        if (equal_pos != std::string::npos) {
            std::string lhs = trim(for_phrase->initializer.substr(0, equal_pos));
            const bool is_mutable = lhs.rfind("mutable ", 0) == 0;
            if (is_mutable) {
                lhs = trim(lhs.substr(8));
            } else if (lhs.rfind("const ", 0) == 0) {
                lhs = trim(lhs.substr(6));
            }

            const size_t name_start = lhs.find_last_of(" \t");
            if (name_start != std::string::npos) {
                const std::string type_name = trim(lhs.substr(0, name_start));
                const std::string name = trim(lhs.substr(name_start + 1));
                if (!type_name.empty() && !name.empty() && isKnownTypeRefString(context.symbols, type_name)) {
                    declareVariable(context, name, normalizeTypeName(type_name), is_mutable, for_phrase);
                }
            }
        } else {
            validateExpression(for_phrase, parseExpressionIR(for_phrase->initializer, phraseStartLocation(for_phrase)), context);
        }
    }

    if (!for_phrase->condition.empty()) {
        validateExpression(for_phrase, parseExpressionIR(for_phrase->condition, phraseStartLocation(for_phrase)), context);
    }

    if (!for_phrase->update.empty()) {
        validateExpression(for_phrase, parseExpressionIR(for_phrase->update, phraseStartLocation(for_phrase)), context);
    }

    ++context.loop_depth;
    analyzePhraseList(for_phrase->nested_phrases, context);
    --context.loop_depth;
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
    for (const auto& parameter : function_phrase->template_parameters) {
        if (context.symbols.known_types.find(parameter.name) != context.symbols.known_types.end()
            || !context.template_type_parameters.insert(parameter.name).second) {
            throw semanticError(function_phrase, "duplicate template type parameter '" + parameter.name + "'");
        }
        for (const auto& allowed_type : parameter.allowed_types) {
            if (!isKnownTypeRefString(context.symbols, allowed_type)) {
                throw semanticError(function_phrase, "unknown template constraint type '" + allowed_type + "'");
            }
            const SemanticTypeRef allowed_type_ref = parseTypeRef(allowed_type);
            if (templateConstraintRequiresPrimitivePrefix(allowed_type, context.symbols) && !allowed_type_ref.is_primitive) {
                throw semanticError(function_phrase, "template constraint '" + allowed_type + "' must use the 'primitive' keyword");
            }
        }
    }

    if (isTypeUnion(context.symbols, function_phrase->return_type)
        || !isKnownTypeRefString(context.symbols, function_phrase->return_type.empty() ? "void" : function_phrase->return_type, context.template_type_parameters)) {
        throw semanticError(function_phrase, "unknown function return type '" + function_phrase->return_type + "' in phrase: " + joinPhraseText(function_phrase));
    }

    const std::string previous_return_type = context.current_function_return_type;
    context.current_function_return_type = return_type;
    const bool previous_function_is_unsafe = context.current_function_is_unsafe;
    context.current_function_is_unsafe = function_phrase->is_unsafe;
    enterScope(context);

    analyzePhraseList(function_phrase->nested_phrases, context);

    leaveScope(context);
    context.current_function_is_unsafe = previous_function_is_unsafe;
    context.current_function_return_type = previous_return_type;
    for (const auto& parameter : function_phrase->template_parameters) {
        context.template_type_parameters.erase(parameter.name);
    }
}

static void analyzeVariableDeclaration(
    const std::shared_ptr<ParsedVariableDeclaration>& variable_phrase,
    ValidationContext& context
) {
    if (!variable_phrase) {
        return;
    }

    const std::string variable_type = normalizeTypeName(variable_phrase->type_name);
    if (isTypeUnion(context.symbols, variable_phrase->type_name)
        || !isKnownTypeRefString(context.symbols, variable_phrase->type_name, context.template_type_parameters)) {
        throw semanticError(variable_phrase, "unknown variable type '" + variable_phrase->type_name + "' in phrase: " + joinPhraseText(variable_phrase));
    }

    if (containsPointerType(parseTypeRef(variable_phrase->type_name)) && !variable_phrase->is_unsafe && context.unsafe_depth == 0) {
        throw semanticError(variable_phrase, "pointer type '" + variable_phrase->type_name + "' must be declared 'unsafe' or used inside an unsafe block/function in phrase: " + joinPhraseText(variable_phrase));
    }

    if (trim(variable_phrase->initializer).empty()) {
        throw semanticError(variable_phrase, "variable must have an initializer in phrase: " + joinPhraseText(variable_phrase));
    }

    if (variable_phrase->is_unsafe) {
        ++context.unsafe_depth;
    }

    SemanticExpressionIR initializer_expr = parseExpressionIR(variable_phrase->initializer, phraseStartLocation(variable_phrase));
    validateExpression(variable_phrase, initializer_expr, context);
    validateAssignmentCompatibility(variable_phrase, variable_type, initializer_expr, context);

    if (variable_phrase->is_unsafe) {
        --context.unsafe_depth;
    }

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

    std::vector<std::string> declared_template_parameters;
    for (const auto& parameter : type_phrase->template_parameters) {
        if (context.symbols.known_types.find(parameter.name) != context.symbols.known_types.end()
            || !context.template_type_parameters.insert(parameter.name).second) {
            throw semanticError(type_phrase, "duplicate template type parameter '" + parameter.name + "'");
        }
        declared_template_parameters.push_back(parameter.name);
    }
    const auto release_template_parameters = [&]() {
        for (const auto& name : declared_template_parameters) {
            context.template_type_parameters.erase(name);
        }
    };

    if (!isKnownTypeRefString(context.symbols, type_phrase->base_type, context.template_type_parameters)) {
        release_template_parameters();
        throw semanticError(type_phrase, "unknown base type '" + type_phrase->base_type + "' in phrase: " + joinPhraseText(type_phrase));
    }

    if (!type_phrase->template_parameters.empty()
        && (!type_phrase->nested_phrases.empty())) {
        release_template_parameters();
        throw semanticError(type_phrase, "generic type '" + type_phrase->name + "' cannot declare contracts or member functions");
    }

    const bool has_body = !type_phrase->nested_phrases.empty();
    if (!has_body) {
        release_template_parameters();
        return;
    }

    enterScope(context);
    declareVariable(context, type_phrase->name, normalizeTypeName(type_phrase->base_type), false, type_phrase);

    for (const auto& nested : type_phrase->nested_phrases) {
        if (!nested) {
            continue;
        }

        if (nested->kind == ParsedPhraseKind::RequiresClause) {
            auto clause = std::dynamic_pointer_cast<ParsedRequiresClause>(nested);
            if (clause) {
                validateExpression(clause, parseExpressionIR(clause->expression, phraseStartLocation(clause)), context);
            }
        } else if (nested->kind == ParsedPhraseKind::EnsuresClause) {
            auto clause = std::dynamic_pointer_cast<ParsedEnsuresClause>(nested);
            if (clause) {
                validateExpression(clause, parseExpressionIR(clause->expression, phraseStartLocation(clause)), context);
            }
        } else if (nested->kind == ParsedPhraseKind::Function) {
            analyzeFunction(std::dynamic_pointer_cast<ParsedFunction>(nested), context);
        }
    }

    leaveScope(context);
    release_template_parameters();
}

static void analyzeStructDefinition(
    const std::shared_ptr<ParsedStructDefinition>& struct_phrase,
    ValidationContext& context
) {
    if (!struct_phrase) {
        return;
    }

    std::vector<std::string> declared_template_parameters;
    for (const auto& parameter : struct_phrase->template_parameters) {
        if (context.symbols.known_types.find(parameter.name) != context.symbols.known_types.end()
            || !context.template_type_parameters.insert(parameter.name).second) {
            throw semanticError(struct_phrase, "duplicate template type parameter '" + parameter.name + "'");
        }
        declared_template_parameters.push_back(parameter.name);
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

        if (!isKnownTypeRefString(context.symbols, field->type_name, context.template_type_parameters)) {
            throw semanticError(field, "unknown struct field type '" + field->type_name + "' in phrase: " + joinPhraseText(field));
        }

        if (field_names.find(field->name) != field_names.end()) {
            throw semanticError(field, "duplicate struct field '" + field->name + "' in phrase: " + joinPhraseText(field));
        }

        field_names.insert(field->name);
    }

    for (const auto& name : declared_template_parameters) {
        context.template_type_parameters.erase(name);
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

static void analyzeUnionDefinition(
    const std::shared_ptr<ParsedUnionDefinition>& union_phrase,
    ValidationContext& context
) {
    if (!union_phrase || union_phrase->members.empty()) {
        throw semanticError(union_phrase, "union must declare at least one member type");
    }

    for (const auto& member : union_phrase->members) {
        if (!isKnownTypeRefString(context.symbols, member)) {
            throw semanticError(union_phrase, "unknown union member type '" + member + "'");
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
    if (isTypeUnion(context.symbols, parameter_phrase->type_name)
        || !isKnownTypeRefString(context.symbols, parameter_phrase->type_name, context.template_type_parameters)) {
        throw semanticError(parameter_phrase, "unknown parameter type '" + parameter_phrase->type_name + "' in phrase: " + joinPhraseText(parameter_phrase));
    }

    if (containsPointerType(parseTypeRef(parameter_phrase->type_name)) && !context.current_function_is_unsafe) {
        throw semanticError(parameter_phrase, "pointer type '" + parameter_phrase->type_name + "' can only be used as a parameter of an 'unsafe function' in phrase: " + joinPhraseText(parameter_phrase));
    }

    declareVariable(
        context,
        parameter_phrase->name,
        parameter_type,
        false,
        parameter_phrase,
        true
    );
}

static void analyzeCallStatement(
    const std::shared_ptr<ParsedCallStatement>& call_phrase,
    ValidationContext& context
) {
    if (!call_phrase) {
        return;
    }

    const size_t dot_index = call_phrase->name.find('.');
    if (dot_index != std::string::npos) {
        const std::string object_name = call_phrase->name.substr(0, dot_index);
        const std::string method_name = call_phrase->name.substr(dot_index + 1);

        VariableSymbol* symbol = lookupVariable(context, object_name);
        std::string type_name;
        if (symbol) {
            type_name = normalizeTypeName(symbol->type_name);
        } else {
            auto external_it = context.symbols.external_values.find(object_name);
            if (external_it == context.symbols.external_values.end()) {
                throw semanticError(call_phrase, "unknown value '" + object_name + "'");
            }
            type_name = normalizeTypeName(external_it->second.type_name);
        }

        const auto type_methods_it = context.symbols.type_member_functions.find(type_name);
        if (type_methods_it == context.symbols.type_member_functions.end()
            || type_methods_it->second.find(method_name) == type_methods_it->second.end()) {
            throw semanticError(call_phrase, "type '" + type_name + "' has no member function '" + method_name + "'");
        }

        const size_t arity = type_methods_it->second.at(method_name);
        if (arity != call_phrase->arguments.size()) {
            throw semanticError(call_phrase, "member function '" + method_name + "' expects " + std::to_string(arity) + " argument(s), got " + std::to_string(call_phrase->arguments.size()));
        }

        for (const auto& argument : call_phrase->arguments) {
            validateExpression(call_phrase, parseExpressionIR(argument, phraseStartLocation(call_phrase)), context);
        }
        return;
    }

    if (call_phrase->name == "pointer" && context.unsafe_depth == 0) {
        throw semanticError(call_phrase, "'pointer()' can only be called inside an unsafe block or unsafe function");
    }

    if (context.symbols.known_functions.find(call_phrase->name) == context.symbols.known_functions.end()) {
        throw semanticError(call_phrase, "unknown function '" + call_phrase->name + "'");
    } else if (context.symbols.variadic_functions.find(call_phrase->name) == context.symbols.variadic_functions.end()) {
        auto arity = context.symbols.function_parameter_counts.find(call_phrase->name);
        if (arity != context.symbols.function_parameter_counts.end() && arity->second != call_phrase->arguments.size()) {
            throw semanticError(call_phrase, "function '" + call_phrase->name + "' expects " + std::to_string(arity->second) + " argument(s), got " + std::to_string(call_phrase->arguments.size()));
        }
    }

    for (const auto& argument : call_phrase->arguments) {
        validateExpression(call_phrase, parseExpressionIR(argument, phraseStartLocation(call_phrase)), context);
    }

    std::vector<SemanticExpressionIR> arguments;
    for (const auto& argument : call_phrase->arguments) {
        arguments.push_back(parseExpressionIR(argument, phraseStartLocation(call_phrase)));
    }
    validateTemplateCallConstraints(call_phrase, call_phrase->name, arguments, context);
}

// Enforces that a function returning a reference or pointer only ever hands
// back storage that outlives the call. Concretely: the returned reference /
// pointer must alias one of the function's own reference or pointer
// parameters (or a subobject of one). Returning a reference or pointer to a
// locally scoped variable is rejected, because the storage dies when the
// function returns and the caller would be left with a dangling alias.
static void validateReturnedReferenceLifetime(
    const std::shared_ptr<ParsedPhrase>& return_phrase,
    const SemanticExpressionIR& expression,
    ValidationContext& context
) {
    const SemanticTypeRef return_type = parseTypeRef(context.current_function_return_type);
    const bool returns_pointer = return_type.kind == SemanticTypeKind::Pointer;
    const bool returns_reference = return_type.kind == SemanticTypeKind::Reference;
    if (!returns_pointer && !returns_reference) {
        return;
    }

    // Unwrap an explicit `reference(x)` / `pointer(x)` constructor so we look
    // at the storage being aliased rather than the wrapper expression.
    const SemanticExpressionIR* aliased = &expression;
    if (expression.kind == SemanticExpressionKind::Call
        && (expression.operator_symbol == "reference" || expression.operator_symbol == "pointer")
        && expression.children.size() == 1) {
        aliased = &expression.children.front();
    }

    // A call that yields a reference / pointer (e.g. forwarding another
    // function's result) cannot be resolved to a single local here; those
    // callees are checked in their own right, so allow it through.
    const std::optional<std::string> root_identifier = expressionRootIdentifier(*aliased);
    if (!root_identifier.has_value()) {
        return;
    }

    const VariableSymbol* symbol = lookupVariable(context, root_identifier.value());
    if (symbol == nullptr) {
        return;
    }

    const SemanticTypeRef symbol_type = parseTypeRef(symbol->type_name);
    const bool aliases_reference_parameter =
        symbol->is_parameter
        && (symbol_type.kind == SemanticTypeKind::Reference || symbol_type.kind == SemanticTypeKind::Pointer);
    if (aliases_reference_parameter) {
        return;
    }

    const std::string kind_word = returns_pointer ? "pointer" : "reference";
    throw semanticError(
        return_phrase,
        "cannot return a " + kind_word + " to '" + root_identifier.value()
            + "': it is a locally scoped variable and would dangle after the function returns; "
              "a function may only return a " + kind_word
            + " that aliases one of its own reference or pointer parameters"
    );
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
    validateReturnedReferenceLifetime(return_phrase, expression, context);
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
        case ParsedPhraseKind::UnionDefinition:
            analyzeUnionDefinition(std::dynamic_pointer_cast<ParsedUnionDefinition>(phrase), context);
            break;
        case ParsedPhraseKind::ImportStatement:
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
            ++context.loop_depth;
            analyzeControlFlowBody(phrase, context);
            --context.loop_depth;
            break;
        case ParsedPhraseKind::ForStatement:
            analyzeForStatement(std::dynamic_pointer_cast<ParsedForStatement>(phrase), context);
            break;
        case ParsedPhraseKind::UnsafeBlock:
            ++context.unsafe_depth;
            analyzeControlFlowBody(phrase, context);
            --context.unsafe_depth;
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

    symbols.known_functions.insert("pointer");
    symbols.function_parameter_counts["pointer"] = 1;
    symbols.known_functions.insert("reference");
    symbols.function_parameter_counts["reference"] = 1;

    registerRuntimeSymbols(symbols);

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

static SemanticFunctionIR lowerFunction(
    const std::shared_ptr<ParsedFunction>& phrase,
    const SemanticSymbolTable& symbols
);

static void renameIdentifierInExpression(SemanticExpressionIR& expression, const std::string& from, const std::string& to) {
    if (expression.kind == SemanticExpressionKind::Identifier && trim(expression.text) == from) {
        expression.text = to;
    }
    for (auto& child : expression.children) {
        renameIdentifierInExpression(child, from, to);
    }
}

static void renameIdentifierInFunction(SemanticFunctionIR& function_ir, const std::string& from, const std::string& to) {
    for (auto& node : function_ir.variable_declarations) {
        renameIdentifierInExpression(node.initializer, from, to);
    }
    for (auto& node : function_ir.assignments) {
        renameIdentifierInExpression(node.expression, from, to);
    }
    for (auto& node : function_ir.calls) {
        for (auto& argument : node.arguments) {
            renameIdentifierInExpression(argument, from, to);
        }
    }
    for (auto& node : function_ir.returns) {
        renameIdentifierInExpression(node.expression, from, to);
    }
    for (auto& node : function_ir.if_statements) {
        renameIdentifierInExpression(node.condition, from, to);
    }
    for (auto& node : function_ir.while_statements) {
        renameIdentifierInExpression(node.condition, from, to);
    }
    for (auto& node : function_ir.for_statements) {
        renameIdentifierInExpression(node.initializer, from, to);
        renameIdentifierInExpression(node.condition, from, to);
        renameIdentifierInExpression(node.update, from, to);
    }
    for (auto& node : function_ir.else_statements) {
        renameIdentifierInExpression(node.condition, from, to);
    }
}

static SemanticTypeDefinitionIR lowerTypeDefinition(const std::shared_ptr<ParsedTypeDefinition>& phrase, const SemanticSymbolTable& symbols) {
    SemanticTypeDefinitionIR node{};
    node.name = phrase->name;
    for (const auto& parameter : phrase->template_parameters) {
        node.template_parameters.push_back(SemanticGenericParameterIR{parameter.name});
    }
    node.base_type = parseTypeRef(phrase->base_type);
    node.location = phraseStartLocation(phrase);

    for (const auto& nested : phrase->nested_phrases) {
        if (!nested) {
            continue;
        }

        if (nested->kind == ParsedPhraseKind::RequiresClause) {
            auto clause = std::dynamic_pointer_cast<ParsedRequiresClause>(nested);
            if (clause) {
                node.has_requires = true;
                node.requires_clause = parseExpressionIR(clause->expression, phraseStartLocation(clause));
            }
        } else if (nested->kind == ParsedPhraseKind::EnsuresClause) {
            auto clause = std::dynamic_pointer_cast<ParsedEnsuresClause>(nested);
            if (clause) {
                node.has_ensures = true;
                node.ensures_clause = parseExpressionIR(clause->expression, phraseStartLocation(clause));
            }
        } else if (nested->kind == ParsedPhraseKind::Function) {
            auto function_phrase = std::dynamic_pointer_cast<ParsedFunction>(nested);
            if (function_phrase) {
                node.member_functions.push_back(lowerFunction(function_phrase, symbols));
            }
        }
    }

    if (node.has_requires) {
        renameIdentifierInExpression(node.requires_clause, node.name, "value");
    }
    if (node.has_ensures) {
        renameIdentifierInExpression(node.ensures_clause, node.name, "value");
    }
    for (auto& member_function : node.member_functions) {
        renameIdentifierInFunction(member_function, node.name, "value");
    }

    return node;
}

static SemanticStructDefinitionIR lowerStructDefinition(const std::shared_ptr<ParsedStructDefinition>& phrase) {
    SemanticStructDefinitionIR node{};
    node.name = phrase->name;
    for (const auto& parameter : phrase->template_parameters) {
        node.template_parameters.push_back(SemanticGenericParameterIR{parameter.name});
    }
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

// Resolves the C++ integral type that backs an enum. An explicit `: primitive
// <int>` underlying wins; otherwise the smallest unsigned type that can hold the
// number of enumerators is chosen.
static std::string enumUnderlyingCppType(const std::string& raw_base_type, size_t value_count) {
    const std::string name = normalizeTypeName(raw_base_type);
    if (name == "uint8") return "std::uint8_t";
    if (name == "uint16") return "std::uint16_t";
    if (name == "uint32") return "std::uint32_t";
    if (name == "uint64") return "std::uint64_t";
    if (name == "int8") return "std::int8_t";
    if (name == "int16") return "std::int16_t";
    if (name == "int32") return "std::int32_t";
    if (name == "int64") return "std::int64_t";

    if (value_count <= 0xFFull) return "std::uint8_t";
    if (value_count <= 0xFFFFull) return "std::uint16_t";
    if (value_count <= 0xFFFFFFFFull) return "std::uint32_t";
    return "std::uint64_t";
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

    node.underlying_cpp_type = enumUnderlyingCppType(phrase->base_type, node.values.size());
    return node;
}

static SemanticImportIR lowerImportStatement(const std::shared_ptr<ParsedImportStatement>& phrase) {
    SemanticImportIR node{};
    node.path = phrase ? phrase->path : "";
    node.import_kind = SemanticImportKind::Cpp;
    node.location = phraseStartLocation(phrase);
    return node;
}

static void lowerBodyPhrases(
    const std::vector<std::shared_ptr<ParsedPhrase>>& phrases,
    std::vector<SemanticVariableDeclarationIR>& variable_declarations,
    std::vector<SemanticAssignmentIR>& assignments,
    std::vector<SemanticTypeDefinitionIR>& type_definitions,
    std::vector<SemanticStructDefinitionIR>& struct_definitions,
    std::vector<SemanticEnumDefinitionIR>& enum_definitions,
    std::vector<SemanticImportIR>& imports,
    std::vector<SemanticCallIR>& calls,
    std::vector<SemanticReturnIR>& returns,
    std::vector<SemanticIfIR>& if_statements,
    std::vector<SemanticWhileIR>& while_statements,
    std::vector<SemanticForIR>& for_statements,
    std::vector<SemanticElseIR>& else_statements,
    std::vector<SemanticStatementRef>& order,
    const SemanticSymbolTable& symbols,
    std::vector<SemanticTriviaIR>* trivia_sink = nullptr
);

static void lowerBodyPhrases(
    const std::vector<std::shared_ptr<ParsedPhrase>>& phrases,
    std::vector<SemanticVariableDeclarationIR>& variable_declarations,
    std::vector<SemanticAssignmentIR>& assignments,
    std::vector<SemanticTypeDefinitionIR>& type_definitions,
    std::vector<SemanticStructDefinitionIR>& struct_definitions,
    std::vector<SemanticEnumDefinitionIR>& enum_definitions,
    std::vector<SemanticImportIR>& imports,
    std::vector<SemanticCallIR>& calls,
    std::vector<SemanticReturnIR>& returns,
    std::vector<SemanticIfIR>& if_statements,
    std::vector<SemanticWhileIR>& while_statements,
    std::vector<SemanticForIR>& for_statements,
    std::vector<SemanticElseIR>& else_statements,
    std::vector<SemanticStatementRef>& order,
    const SemanticSymbolTable& symbols,
    std::vector<SemanticTriviaIR>* trivia_sink
) {
    for (const auto& phrase : phrases) {
        if (!phrase) {
            continue;
        }

        // Some nested phrases (parameter declarations, contract clauses) share
        // their `source_phrase` with the owning function and never produce a
        // statement; trivia is only attached to phrases that actually emit one.
        const std::shared_ptr<Phrase> source_phrase = phrase->source_phrase;
        const size_t order_size_before_statement = order.size();

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
                auto node = lowerTypeDefinition(std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase), symbols);
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
            case ParsedPhraseKind::ImportStatement: {
                auto import_phrase = std::dynamic_pointer_cast<ParsedImportStatement>(phrase);
                if (import_phrase && import_phrase->import_kind == ParsedImportKind::Cpp) {
                    imports.push_back(lowerImportStatement(import_phrase));
                    order.push_back({SemanticStatementKind::Import, imports.size() - 1});
                }
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
                        imports,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body,
                        symbols,
                        trivia_sink
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
                        imports,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body,
                        symbols,
                        trivia_sink
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
                        imports,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body,
                        symbols,
                        trivia_sink
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
                        imports,
                        calls,
                        returns,
                        if_statements,
                        while_statements,
                        for_statements,
                        else_statements,
                        node.body,
                        symbols,
                        trivia_sink
                    );
                    else_statements.push_back(node);
                    order.push_back({SemanticStatementKind::Else, else_statements.size() - 1});
                break;
            }
            case ParsedPhraseKind::UnsafeBlock: {
                lowerBodyPhrases(
                    phrase->nested_phrases,
                    variable_declarations,
                    assignments,
                    type_definitions,
                    struct_definitions,
                    enum_definitions,
                    imports,
                    calls,
                    returns,
                    if_statements,
                    while_statements,
                    for_statements,
                    else_statements,
                    order,
                    symbols,
                    trivia_sink
                );
                break;
            }
            default:
                break;
        }

        if (trivia_sink == nullptr || !source_phrase || order.size() == order_size_before_statement) {
            continue;
        }

        if (!source_phrase->leading_trivia.empty()) {
            trivia_sink->push_back(makeLeadingTrivia(source_phrase->leading_trivia));
            order.insert(
                order.begin() + static_cast<std::ptrdiff_t>(order_size_before_statement),
                SemanticStatementRef{SemanticStatementKind::Trivia, trivia_sink->size() - 1}
            );
        }

        if (!source_phrase->trailing_comment.empty()) {
            SemanticTriviaIR suffix_trivia{};
            suffix_trivia.suffix = source_phrase->trailing_comment;
            trivia_sink->push_back(suffix_trivia);
            order.push_back({SemanticStatementKind::Trivia, trivia_sink->size() - 1});
        }
    }
}

static void expandUnionMembers(
    const SemanticSymbolTable& symbols,
    const std::string& type_name,
    std::vector<std::string>& expanded_types
) {
    const auto union_it = symbols.union_members.find(normalizeTypeName(type_name));
    if (union_it == symbols.union_members.end()) {
        expanded_types.push_back(type_name);
        return;
    }

    for (const auto& member : union_it->second) {
        expandUnionMembers(symbols, member, expanded_types);
    }
}

static SemanticFunctionIR lowerFunction(
    const std::shared_ptr<ParsedFunction>& phrase,
    const SemanticSymbolTable& symbols
) {
    SemanticFunctionIR function_ir{};
    function_ir.name = phrase->name;
    function_ir.return_type = phrase->return_type.empty() ? parseTypeRef("void") : parseTypeRef(phrase->return_type);
    function_ir.return_value_name = phrase->return_value_name;
    function_ir.has_requires = !trim(phrase->requires_clause).empty();
    function_ir.has_ensures = !trim(phrase->ensures_clause).empty();
    if (function_ir.has_requires) {
        function_ir.requires_clause = parseExpressionIR(phrase->requires_clause, phraseStartLocation(phrase));
    }
    if (function_ir.has_ensures) {
        function_ir.ensures_clause = parseExpressionIR(phrase->ensures_clause, phraseStartLocation(phrase));
    }
    function_ir.tags = phrase->tags;
    function_ir.is_discardable = std::find(function_ir.tags.begin(), function_ir.tags.end(), "discard") != function_ir.tags.end();
    for (const auto& parameter : phrase->template_parameters) {
        SemanticTemplateParameterIR template_parameter{};
        template_parameter.name = parameter.name;
        template_parameter.category = parameter.category;
        for (const auto& allowed_type : parameter.allowed_types) {
            std::vector<std::string> expanded_types;
            expandUnionMembers(symbols, allowed_type, expanded_types);
            for (const auto& expanded_type : expanded_types) {
                template_parameter.allowed_types.push_back(parseTypeRef(expanded_type));
            }
        }
        function_ir.template_parameters.push_back(template_parameter);
    }
    function_ir.location = phraseStartLocation(phrase);
    if (phrase->source_phrase) {
        function_ir.leading_trivia_lines = triviaToLines(phrase->source_phrase->leading_trivia);
        function_ir.trailing_trivia_lines = triviaToLines(phrase->source_phrase->trailing_trivia);
    }

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
        function_ir.imports,
        function_ir.calls,
        function_ir.returns,
        function_ir.if_statements,
        function_ir.while_statements,
        function_ir.for_statements,
        function_ir.else_statements,
        function_ir.body_order,
        symbols,
        &function_ir.trivia
    );

    return function_ir;
}

static void lowerTopLevelPhrase(
    const std::shared_ptr<ParsedPhrase>& phrase,
    SemanticProgram& program,
    const SemanticSymbolTable& symbols
) {
    if (!phrase) {
        return;
    }

    const size_t top_level_order_size_before = program.top_level_order.size();

    switch (phrase->kind) {
        case ParsedPhraseKind::Function:
            program.functions.push_back(lowerFunction(std::dynamic_pointer_cast<ParsedFunction>(phrase), symbols));
            program.top_level_order.push_back({SemanticStatementKind::Function, program.functions.size() - 1});
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
            program.type_definitions.push_back(lowerTypeDefinition(std::dynamic_pointer_cast<ParsedTypeDefinition>(phrase), symbols));
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
        case ParsedPhraseKind::ImportStatement: {
            auto import_phrase = std::dynamic_pointer_cast<ParsedImportStatement>(phrase);
            if (import_phrase && import_phrase->import_kind == ParsedImportKind::Cpp) {
                program.imports.push_back(lowerImportStatement(import_phrase));
                program.top_level_order.push_back({SemanticStatementKind::Import, program.imports.size() - 1});
            }
            break;
        }
        default:
            break;
    }

    // Functions carry their own trivia through SemanticFunctionIR.
    if (phrase->kind == ParsedPhraseKind::Function
        || !phrase->source_phrase
        || program.top_level_order.size() == top_level_order_size_before) {
        return;
    }

    if (!phrase->source_phrase->leading_trivia.empty()) {
        program.trivia.push_back(makeLeadingTrivia(phrase->source_phrase->leading_trivia));
        program.top_level_order.insert(
            program.top_level_order.begin() + static_cast<std::ptrdiff_t>(top_level_order_size_before),
            SemanticStatementRef{SemanticStatementKind::Trivia, program.trivia.size() - 1}
        );
    }

    if (!phrase->source_phrase->trailing_comment.empty()) {
        SemanticTriviaIR suffix_trivia{};
        suffix_trivia.suffix = phrase->source_phrase->trailing_comment;
        program.trivia.push_back(suffix_trivia);
        program.top_level_order.push_back({SemanticStatementKind::Trivia, program.trivia.size() - 1});
    }
}

SemanticProgram buildSemanticProgram(const ParsedPhrases& phrases) {
    const SemanticSymbolTable symbols = collectSymbols(phrases);
    validateTypeUnionMembers(symbols);
    validateSemantics(phrases, symbols);

    SemanticProgram program{};
    program.union_members = symbols.union_members;
    for (const auto& phrase : phrases) {
        lowerTopLevelPhrase(phrase, program, symbols);
    }

    return program;
}

void analyzeParsedPhrases(const ParsedPhrases& phrases) {
    const SemanticSymbolTable symbols = collectSymbols(phrases);
    validateTypeUnionMembers(symbols);
    validateSemantics(phrases, symbols);
}
