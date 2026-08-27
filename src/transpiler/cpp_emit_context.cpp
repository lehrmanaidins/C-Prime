#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../semantic/semantic.cpp"

struct CppSourceMapEntry {
    size_t cpp_line;
    size_t cprime_line;
    size_t cprime_column;
    std::string note;
};

struct CppEmitResult {
    std::string code;
    std::vector<CppSourceMapEntry> source_map;
    std::vector<std::string> diagnostics;
};

struct CppEmitContext {
    std::unordered_set<std::string> required_headers;
    std::unordered_map<std::string, SemanticTypeRef> type_aliases;
    std::unordered_set<std::string> struct_types;
    std::vector<CppSourceMapEntry> source_map;
    size_t current_cpp_line = 1;

    void pushLine(const SourceLocation& loc, const std::string& note) {
        source_map.push_back({current_cpp_line, loc.line, loc.column, note});
    }
};

static std::string sanitizeIdentifier(const std::string& name) {
    static const std::set<std::string> cpp_keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
        "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept",
        "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return",
        "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
        "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int",
        "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
        "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "requires", "return",
        "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "template",
        "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned",
        "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };

    if (cpp_keywords.find(name) != cpp_keywords.end()) {
        return name + "_cp";
    }

    return name;
}

static std::string mapPrimitiveType(const std::string& type_name, CppEmitContext& context) {
    if (type_name == "bool") return "bool";
    if (type_name == "char8") return "char8_t";
    if (type_name == "char16") return "char16_t";
    if (type_name == "char32") return "char32_t";
    if (type_name == "int8" || type_name == "int16" || type_name == "int32" || type_name == "int64"
        || type_name == "uint8" || type_name == "uint16" || type_name == "uint32" || type_name == "uint64") {
        context.required_headers.insert("<cstdint>");
    }

    if (type_name == "int8") return "int8_t";
    if (type_name == "int16") return "int16_t";
    if (type_name == "int32") return "int32_t";
    if (type_name == "int64") return "int64_t";
    if (type_name == "uint8") return "uint8_t";
    if (type_name == "uint16") return "uint16_t";
    if (type_name == "uint32") return "uint32_t";
    if (type_name == "uint64") return "uint64_t";
    if (type_name == "float32") return "float";
    if (type_name == "float64") return "double";
    if (type_name == "void") return "void";

    return sanitizeIdentifier(type_name);
}

static std::string indent(size_t depth) {
    return std::string(depth * 4, ' ');
}

static void appendLine(std::string& output, CppEmitContext& context, const std::string& line) {
    output += line;
    output += "\n";
    ++context.current_cpp_line;
}
