#pragma once

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../embedded/resources.hpp"

struct CppImportTypeInfo {
    std::string cprime_name;
    bool is_cprime_interface_type = false;
};

static std::string stripCppLineComment(const std::string& line) {
    bool escaped = false;
    char quote = '\0';

    for (size_t i = 0; i + 1 < line.size(); ++i) {
        const char ch = line[i];
        if (quote != '\0') {
            if (ch == '\\' && !escaped) {
                escaped = true;
                continue;
            }
            if (ch == quote && !escaped) {
                quote = '\0';
            }
            escaped = false;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            quote = ch;
            continue;
        }

        if (ch == '/' && line[i + 1] == '/') {
            return line.substr(0, i);
        }
    }

    return line;
}

static CppImportTypeInfo normalizeCppImportType(const std::string& raw_type) {
    std::string type = trim(raw_type);

    for (const std::string prefix : {"static ", "inline ", "constexpr ", "const "}) {
        while (type.rfind(prefix, 0) == 0) {
            type = trim(type.substr(prefix.size()));
        }
    }

    while (!type.empty() && (type.back() == '&' || type.back() == '*')) {
        type = trim(type.substr(0, type.size() - 1));
    }

    if (type == "void") {
        return CppImportTypeInfo{"void", true};
    }

    if (type == "bool") {
        return CppImportTypeInfo{"bool", true};
    }

    const std::string cprime_prefix = "cprime::";
    if (type.rfind(cprime_prefix, 0) == 0) {
        type = type.substr(cprime_prefix.size());
        return CppImportTypeInfo{type, true};
    }

    return CppImportTypeInfo{type, false};
}

static std::string normalizeCppTypeName(const std::string& raw_type) {
    return normalizeCppImportType(raw_type).cprime_name;
}

static bool isCppImportParameterCPrimeCompatible(const std::string& parameter) {
    const std::string trimmed = trim(parameter);
    if (trimmed.empty() || trimmed == "void") {
        return true;
    }

    const size_t last_space = trimmed.find_last_of(" \t");
    const std::string type_text = last_space == std::string::npos
        ? trimmed
        : trim(trimmed.substr(0, last_space));

    return normalizeCppImportType(type_text).is_cprime_interface_type;
}

static size_t countCppParameters(const std::string& raw_parameters) {
    const std::string parameters = trim(raw_parameters);
    if (parameters.empty() || parameters == "void") {
        return 0;
    }

    return splitTopLevel(parameters, ',').size();
}

static bool allCppImportParametersCPrimeCompatible(const std::string& raw_parameters) {
    const std::string parameters = trim(raw_parameters);
    if (parameters.empty() || parameters == "void") {
        return true;
    }

    for (const auto& parameter : splitTopLevel(parameters, ',')) {
        if (!isCppImportParameterCPrimeCompatible(parameter)) {
            return false;
        }
    }

    return true;
}

static bool includesCPrimeRuntimeHeader(const std::vector<std::string>& lines) {
    const std::regex include_regex(R"(^\s*#\s*include\s*[<\"](?:src/runtime/)?c-prime\.hpp[>\"])");

    for (const auto& line : lines) {
        if (std::regex_search(stripCppLineComment(line), include_regex)) {
            return true;
        }
    }

    return false;
}

static bool isCppVariadicTemplateLine(const std::string& line) {
    static const std::regex template_regex(R"(^\s*template\s*<.*\.\.\..*>\s*$)");
    return std::regex_search(line, template_regex);
}

static void scanCppSymbolLines(const std::vector<std::string>& lines, SemanticSymbolTable& symbols) {
    const std::regex type_regex(R"(^\s*(?:struct|class)\s+([A-Za-z_]\w*)\b|^\s*enum\s+(?:class\s+)?([A-Za-z_]\w*)\b|^\s*using\s+([A-Za-z_]\w*)\s*=|^\s*typedef\s+.*\s+([A-Za-z_]\w*)\s*;)");
    const std::regex function_regex(R"(^\s*(?:static\s+|inline\s+|constexpr\s+|const\s+)*([A-Za-z_][\w:<>\s,&*]*)\s+([A-Za-z_]\w*)\s*\(([^()]*)\)\s*(?:const\s*)?(?:\{|;))");
    const std::regex value_regex(R"(^\s*(?:static\s+|inline\s+|constexpr\s+|const\s+)*([A-Za-z_][\w:<>\s,&*]*)\s+([A-Za-z_]\w*)\s*(?:=|;))");

    bool pending_variadic_template = false;
    bool in_macro_definition = false;

    for (std::string line : lines) {
        line = stripCppLineComment(line);

        // Preprocessor directives, and the continued body lines of a multi-line
        // #define, are not real declarations. Skipping them keeps a `struct T {`
        // that appears inside a macro definition from being registered as a type.
        const std::string trimmed_line = trim(line);
        if (in_macro_definition) {
            in_macro_definition = !trimmed_line.empty() && trimmed_line.back() == '\\';
            continue;
        }
        if (!trimmed_line.empty() && trimmed_line.front() == '#') {
            in_macro_definition = trimmed_line.back() == '\\';
            continue;
        }

        if (isCppVariadicTemplateLine(line)) {
            pending_variadic_template = true;
            continue;
        }

        const bool is_variadic_declaration = pending_variadic_template;
        pending_variadic_template = false;

        std::smatch match;
        if (std::regex_search(line, match, type_regex)) {
            for (size_t i = 1; i < match.size(); ++i) {
                if (match[i].matched) {
                    const std::string type_name = match[i].str();
                    symbols.known_types.insert({type_name, TypeSymbol{TypeSymbolKind::Struct, ""}});
                    break;
                }
            }
        }

        if (std::regex_search(line, match, function_regex)) {
            const CppImportTypeInfo return_type = normalizeCppImportType(match[1].str());
            const std::string function_name = match[2].str();
            const std::string parameters = match[3].str();
            if (function_name == "if" || function_name == "for" || function_name == "while" || function_name == "switch") {
                continue;
            }

            if (is_variadic_declaration && parameters.find("...") != std::string::npos && return_type.is_cprime_interface_type) {
                symbols.known_functions.insert(function_name);
                symbols.function_return_types[function_name] = return_type.cprime_name;
                symbols.variadic_functions.insert(function_name);
                continue;
            }

            if (return_type.is_cprime_interface_type && allCppImportParametersCPrimeCompatible(parameters)) {
                symbols.known_functions.insert(function_name);
                symbols.function_return_types[function_name] = return_type.cprime_name;
                symbols.function_parameter_counts[function_name] = countCppParameters(parameters);
            }
            continue;
        }

        if (line.find('(') == std::string::npos && std::regex_search(line, match, value_regex)) {
            const CppImportTypeInfo value_type = normalizeCppImportType(match[1].str());
            const std::string value_name = match[2].str();
            if (value_type.is_cprime_interface_type) {
                symbols.external_values.insert({value_name, VariableSymbol{value_type.cprime_name, false}});
            }
        }
    }
}

static std::vector<std::string> splitTextLines(std::string_view text) {
    std::vector<std::string> lines{};
    std::istringstream stream{std::string(text)};
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

static std::vector<std::string> readAllLines(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return splitTextLines(buffer.str());
}

static void registerCppImportSymbols(const std::string& path, SemanticSymbolTable& symbols) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return;
    }
    file.close();

    const std::vector<std::string> lines = readAllLines(path);

    if (!includesCPrimeRuntimeHeader(lines)) {
        throw std::runtime_error("Import error: C++ import '" + path + "' must include c-prime.hpp");
    }

    scanCppSymbolLines(lines, symbols);
}

// Scans the always-included runtime headers themselves, so builtins like
// print/println and library types like reference/pointer are ordinary symbols
// provided by the runtime rather than compiler keywords.
static void registerRuntimeSymbols(SemanticSymbolTable& symbols) {
    scanCppSymbolLines(splitTextLines(embedded::runtime_hpp()), symbols);
    scanCppSymbolLines(splitTextLines(embedded::io_hpp()), symbols);
    scanCppSymbolLines(splitTextLines(embedded::memory_hpp()), symbols);
}