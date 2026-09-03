
#ifndef CPRIME_RUNTIME_IO_HPP
#define CPRIME_RUNTIME_IO_HPP

// C-Prime Standard Library.

#include <optional>
#include <type_traits>
#include <iostream>

template <typename... Args>
void print(const Args&... args) {
    (std::cout << ... << args);
}

template <typename... Args>
void println(const Args&... args) {
    print(args...);
    std::cout << '\n';
}

#endif  // CPRIME_RUNTIME_IO_HPP
