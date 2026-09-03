#pragma once

#include <cstddef>
#include <string_view>

// The transpiler is distributed as a single binary. The lexer term lists, the
// C-Prime prelude, and the C++ runtime headers are embedded here at compile time
// with `#embed` so nothing has to be read from disk relative to the working
// directory. `#embed` resolves these paths against the compiler include paths;
// CMake adds the repository root as one.
//
// Each array carries a leading sentinel byte so the initializer is well-formed
// even if a file is empty (`{0,}` is a valid aggregate initializer); the
// string_view then starts one byte in and never includes the sentinel.

namespace embedded {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc23-extensions"

inline constexpr unsigned char separators_txt_data[] = {0,
#embed "../lists/separators.txt"
};

inline constexpr unsigned char delimiters_txt_data[] = {0,
#embed "../lists/delimiters.txt"
};

inline constexpr unsigned char keywords_txt_data[] = {0,
#embed "../lists/keywords.txt"
};

inline constexpr unsigned char primitives_txt_data[] = {0,
#embed "../lists/primitives.txt"
};

inline constexpr unsigned char operators_txt_data[] = {0,
#embed "../lists/operators.txt"
};

inline constexpr unsigned char runtime_hpp_data[] = {0,
#embed "../runtime/c-prime.hpp"
};

inline constexpr unsigned char std_hpp_data[] = {0,
#embed "../runtime/std.hpp"
};

inline constexpr unsigned char std_hprime_data[] = {0,
#embed "../runtime/std.hprime"
};

#pragma clang diagnostic pop

// Runtime accessors: the `reinterpret_cast` from the byte array keeps these out
// of constant evaluation, which is fine — they are read once at startup.
inline std::string_view asText(const unsigned char* data, std::size_t size) {
    return {reinterpret_cast<const char*>(data) + 1, size - 1};
}

inline std::string_view separators_txt() { return asText(separators_txt_data, sizeof(separators_txt_data)); }
inline std::string_view delimiters_txt() { return asText(delimiters_txt_data, sizeof(delimiters_txt_data)); }
inline std::string_view keywords_txt() { return asText(keywords_txt_data, sizeof(keywords_txt_data)); }
inline std::string_view primitives_txt() { return asText(primitives_txt_data, sizeof(primitives_txt_data)); }
inline std::string_view operators_txt() { return asText(operators_txt_data, sizeof(operators_txt_data)); }
inline std::string_view runtime_hpp() { return asText(runtime_hpp_data, sizeof(runtime_hpp_data)); }
inline std::string_view std_hpp() { return asText(std_hpp_data, sizeof(std_hpp_data)); }
inline std::string_view std_hprime() { return asText(std_hprime_data, sizeof(std_hprime_data)); }

}  // namespace embedded
