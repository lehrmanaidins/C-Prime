
#ifndef CPRIME_RUNTIME_TYPES_HPP
#define CPRIME_RUNTIME_TYPES_HPP

// C-Prime Standard Library.

#include <optional>
#include <type_traits>

namespace cprime {
    template <typename T>
    using optional = std::optional<T>;

    // A non-owning alias to an existing object. Non-null by construction.
    // Lifetime and mutability guarantees are enforced by the C-Prime front
    // end; at runtime this is nothing more than a checked pointer that
    // auto-dereferences on use.
    template <typename T>
    struct reference {
        T* target;

        constexpr explicit reference(T& value) noexcept : target(&value) {}
        constexpr reference(const reference&) noexcept = default;
        constexpr reference& operator=(const reference&) noexcept = default;
        reference(T&&) = delete;

        // Allow reference<X> -> reference<const X> (loss of write-through).
        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        constexpr reference(const reference<U>& other) noexcept : target(other.target) {}

        constexpr operator T&() const noexcept { return *target; }
        constexpr T& get() const noexcept { return *target; }
    };

    template <typename T>
    reference(T&) -> reference<T>;

    // An unsafe, low-level address. The C-Prime front end only permits its
    // construction inside an unsafe context.
    template <typename T>
    struct pointer {
        T* address;

        constexpr explicit pointer(T& value) noexcept : address(&value) {}
        constexpr explicit pointer(T* value) noexcept : address(value) {}
        constexpr pointer(const pointer&) noexcept = default;
        constexpr pointer& operator=(const pointer&) noexcept = default;
        pointer(T&&) = delete;

        // Allow pointer<X> -> pointer<const X> (loss of write-through).
        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        constexpr pointer(const pointer<U>& other) noexcept : address(other.address) {}

        constexpr operator T&() const noexcept { return *address; }
        constexpr T& get() const noexcept { return *address; }
        constexpr T* raw() const noexcept { return address; }
    };

    template <typename T>
    pointer(T&) -> pointer<T>;
}

template <typename... Args>
void print(const Args&... args) {
    (std::cout << ... << args);
}

template <typename... Args>
void println(const Args&... args) {
    print(args...);
    std::cout << '\n';
}

#endif  // CPRIME_RUNTIME_TYPES_HPP
