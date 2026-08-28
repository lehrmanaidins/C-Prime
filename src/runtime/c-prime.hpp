#pragma once

#include <array>
#include <cstdint>
#include <stdfloat>
#include <iostream>
#include <tuple>

namespace cprime {
    using uint8 = std::uint8_t;
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;
    using uint64 = std::uint64_t;

    using int8 = std::int8_t;
    using int16 = std::int16_t;
    using int32 = std::int32_t;
    using int64 = std::int64_t;

    using float32 = float;
    using float64 = double;

    using char8 = char8_t;
    using char16 = char16_t;
    using char32 = char32_t;

    template <typename T, std::size_t Size>
    using array = std::array<T, Size>;

    template <typename... Elements>
    using tuple = std::tuple<Elements...>;

    template <typename T>
    using reference = T&;

    template <typename T>
    using pointer = T*;

    inline void print_char_code_point(std::uint32_t code_point) {
        if (code_point <= 0x7F) {
            std::cout.put(static_cast<char>(code_point));
        } else if (code_point <= 0x7FF) {
            std::cout.put(static_cast<char>(0xC0 | (code_point >> 6)));
            std::cout.put(static_cast<char>(0x80 | (code_point & 0x3F)));
        } else if (code_point <= 0xFFFF) {
            std::cout.put(static_cast<char>(0xE0 | (code_point >> 12)));
            std::cout.put(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
            std::cout.put(static_cast<char>(0x80 | (code_point & 0x3F)));
        } else {
            std::cout.put(static_cast<char>(0xF0 | (code_point >> 18)));
            std::cout.put(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
            std::cout.put(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
            std::cout.put(static_cast<char>(0x80 | (code_point & 0x3F)));
        }
    }

    inline void print_value(char8 value) {
        print_char_code_point(static_cast<std::uint8_t>(value));
    }

    inline void print_value(char16 value) {
        print_char_code_point(static_cast<std::uint16_t>(value));
    }

    inline void print_value(char32 value) {
        print_char_code_point(static_cast<std::uint32_t>(value));
    }

    template <typename T>
    void print_value(const T& value) {
        std::cout << value;
    }
}

template <typename T>
cprime::pointer<T> pointer(T& value) {
    return &value;
}

template <typename T>
cprime::reference<T> reference(T& value) {
    return value;
}

template <typename... Args>
void print(const Args&... args) {
    (cprime::print_value(args), ...);
}

template <typename... Args>
void println(const Args&... args) {
    print(args...);
    std::cout << std::endl;
}
