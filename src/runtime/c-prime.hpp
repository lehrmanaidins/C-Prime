#pragma once

#include <array>
#include <cstdint>
#include <stdfloat>
#include <iostream>
#include <tuple>
#include <vector>

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

    template <typename T>
    using list = std::vector<T>;

    template <typename... Elements>
    using tuple = std::tuple<Elements...>;
}

template <typename... Args>
void print(const Args&... args) {
    (std::cout << ... << args);
}

template <typename... Args>
void println(const Args&... args) {
    (std::cout << ... << args);
    std::cout << std::endl;
}
