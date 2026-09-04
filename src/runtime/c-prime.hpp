
#ifndef CPRIME_RUNTIME_HPP
#define CPRIME_RUNTIME_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
// #include <limits>
#include <numeric>
// #include <stdexcept>
// #include <string>
#include <type_traits>
#include <utility>

// The C-Prime primitive types are strong wrappers over a standard scalar. These
// macros forward the scalar's operators onto the wrapper so a primitive (and any
// domain type built from one) behaves like its underlying type while staying a
// distinct type. Value-producing operators keep the wrapper type; a cast back to
// the underlying scalar avoids integer-promotion warnings under strict builds.
// Removes one layer of enclosing parentheses. A composite underlying type such
// as `std::array<T, Size>` carries a top-level comma, so it must be passed to
// the macros parenthesised; this unwraps it back into a usable type name.
#define CPRIME_EXPAND(...) __VA_ARGS__

// Picks a streamable representation for a wrapped value. The character-sized
// integer types are promoted so they print as numbers; a nested C-Prime wrapper
// (when a domain type's underlying is another `type`) is left alone and streams
// through its own `operator<<`. Being a template, the untaken `if constexpr`
// branch is discarded, so this stays well-formed for every underlying type.
template <typename CPrimeStreamValue>
constexpr auto cprimeStreamable(const CPrimeStreamValue& value) {
    if constexpr (std::is_same_v<CPrimeStreamValue, unsigned char>
        || std::is_same_v<CPrimeStreamValue, signed char>
        || std::is_same_v<CPrimeStreamValue, char>) {
        return static_cast<int>(value);
    } else {
        return value;
    }
}

// The wrapper-member macros splice `Underlying` into type positions (`Underlying&&`,
// `static_cast<Underlying>`), where clang-tidy's bugprone-macro-parentheses wants
// parentheses that would be ill-formed. Callers always pass a single type name or a
// parenthesised composite unwrapped via CPRIME_EXPAND, so the check is suppressed here.
// NOLINTBEGIN(bugprone-macro-parentheses)
#define CPRIME_COMMON_CONSTRUCTORS(T, Underlying) \
    private: \
    Underlying value; \
    public: \
    T(const Underlying& value_arg) = delete; \
    constexpr T(Underlying&& value_arg) : value(std::forward<Underlying>(value_arg)) {} \
    [[nodiscard]] explicit constexpr operator Underlying() const { return value; } \

#define CPRIME_EQUALITY_OPERATORS(T) \
    [[nodiscard]] constexpr auto operator==(T other) const -> bool { return value == other.value; } \
    [[nodiscard]] constexpr auto operator!=(T other) const -> bool { return value != other.value; }

#define CPRIME_ORDERED_OPERATORS(T) \
    [[nodiscard]] constexpr auto operator<(T other) const -> bool { return value < other.value; } \
    [[nodiscard]] constexpr auto operator<=(T other) const -> bool { return value <= other.value; } \
    [[nodiscard]] constexpr auto operator>(T other) const -> bool { return value > other.value; } \
    [[nodiscard]] constexpr auto operator>=(T other) const -> bool { return value >= other.value; }

#define CPRIME_COMPARISON_OPERATORS(T) \
    CPRIME_EQUALITY_OPERATORS(T) \
    CPRIME_ORDERED_OPERATORS(T)

#define CPRIME_ARITHMETIC_INTEGER_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator+(T other) const -> T { return T(static_cast<Underlying>(std::add_sat(value, other.value))); } \
    [[nodiscard]] constexpr auto operator-(T other) const -> T { return T(static_cast<Underlying>(std::sub_sat(value, other.value))); } \
    [[nodiscard]] constexpr auto operator*(T other) const -> T { return T(static_cast<Underlying>(std::mul_sat(value, other.value))); } \
    [[nodiscard]] constexpr auto operator/(T other) const -> T { return T(static_cast<Underlying>(std::div_sat(value, other.value))); } \
    [[nodiscard]] constexpr auto operator%(T other) const -> T { return T(static_cast<Underlying>(value % other.value)); } \
    constexpr auto operator+=(T other) -> T& { value = static_cast<Underlying>(std::add_sat(value, other.value)); return *this; } \
    constexpr auto operator-=(T other) -> T& { value = static_cast<Underlying>(std::sub_sat(value, other.value)); return *this; } \
    constexpr auto operator*=(T other) -> T& { value = static_cast<Underlying>(std::mul_sat(value, other.value)); return *this; } \
    constexpr auto operator/=(T other) -> T& { value = static_cast<Underlying>(std::div_sat(value, other.value)); return *this; } \
    constexpr auto operator%=(T other) -> T& { value = static_cast<Underlying>(value % other.value); return *this; } \

#define CPRIME_ARITHMETIC_SIGNED_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator+() const -> T { return *this; } \
    [[nodiscard]] constexpr auto operator-() const -> T { return T(static_cast<Underlying>(-value)); }

#define CPRIME_ARITHMETIC_FLOATING_POINT_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator+(T other) const -> T { return T(static_cast<Underlying>(value + other.value)); } \
    [[nodiscard]] constexpr auto operator-(T other) const -> T { return T(static_cast<Underlying>(value - other.value)); } \
    [[nodiscard]] constexpr auto operator*(T other) const -> T { return T(static_cast<Underlying>(value * other.value)); } \
    [[nodiscard]] constexpr auto operator/(T other) const -> T { return T(static_cast<Underlying>(value / other.value)); } \
    [[nodiscard]] constexpr auto operator%(T other) const -> T { return T(static_cast<Underlying>(std::fmod(value, other.value))); } \
    constexpr auto operator+=(T other) -> T& { value = static_cast<Underlying>(value + other.value); return *this; } \
    constexpr auto operator-=(T other) -> T& { value = static_cast<Underlying>(value - other.value); return *this; } \
    constexpr auto operator*=(T other) -> T& { value = static_cast<Underlying>(value * other.value); return *this; } \
    constexpr auto operator/=(T other) -> T& { value = static_cast<Underlying>(value / other.value); return *this; } \
    constexpr auto operator%=(T other) -> T& { value = static_cast<Underlying>(std::fmod(value, other.value)); return *this; } \

#define CPRIME_BITWISE_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator&(T other) const -> T { return T(static_cast<Underlying>(value & other.value)); } \
    [[nodiscard]] constexpr auto operator|(T other) const -> T { return T(static_cast<Underlying>(value | other.value)); } \
    [[nodiscard]] constexpr auto operator^(T other) const -> T { return T(static_cast<Underlying>(value ^ other.value)); } \
    [[nodiscard]] constexpr auto operator~() const -> T { return T(static_cast<Underlying>(~value)); } \
    [[nodiscard]] constexpr auto operator<<(T other) const -> T { return T(static_cast<Underlying>(value << other.value)); } \
    [[nodiscard]] constexpr auto operator>>(T other) const -> T { return T(static_cast<Underlying>(value >> other.value)); } \
    constexpr auto operator&=(T other) -> T& { value = static_cast<Underlying>(value & other.value); return *this; } \
    constexpr auto operator|=(T other) -> T& { value = static_cast<Underlying>(value | other.value); return *this; } \
    constexpr auto operator^=(T other) -> T& { value = static_cast<Underlying>(value ^ other.value); return *this; } \
    constexpr auto operator<<=(T other) -> T& { value = static_cast<Underlying>(value << other.value); return *this; } \
    constexpr auto operator>>=(T other) -> T& { value = static_cast<Underlying>(value >> other.value); return *this; }

#define CPRIME_BOOLEAN_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator&&(T other) const -> T { return T(value && other.value); } \
    [[nodiscard]] constexpr auto operator||(T other) const -> T { return T(value || other.value); } \
    [[nodiscard]] constexpr auto operator!() const -> T { return T(!value); }

// The *_MEMBERS macros are struct-body fragments: operators plus any extra
// member functions passed as trailing arguments. They are used by the *_TYPE
// macros below and are also emitted directly into a hand-written wrapper struct
// for C-Prime `type` definitions, so that struct's constructor can carry
// contracts and its `<<` can delegate to the wrapped value's own streaming.
#define CPRIME_UNSIGNED_INTEGER_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_CONSTRUCTORS(T, Underlying) \
    template <typename UnsignedIntegerLiteral> requires std::is_integral_v<UnsignedIntegerLiteral> \
    constexpr T(UnsignedIntegerLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_INTEGER_OPERATORS(T, Underlying) \
    CPRIME_BITWISE_OPERATORS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << cprimeStreamable(self.value); } \
    __VA_ARGS__

#define CPRIME_SIGNED_INTEGER_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_CONSTRUCTORS(T, Underlying) \
    template <typename SignedIntegerLiteral> requires std::is_integral_v<SignedIntegerLiteral> \
    constexpr T(SignedIntegerLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_INTEGER_OPERATORS(T, Underlying) \
    CPRIME_ARITHMETIC_SIGNED_OPERATORS(T, Underlying) \
    CPRIME_BITWISE_OPERATORS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << cprimeStreamable(self.value); } \
    __VA_ARGS__

#define CPRIME_FLOATING_POINT_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_CONSTRUCTORS(T, Underlying) \
    template <typename FloatingPointLiteral> requires std::is_floating_point_v<FloatingPointLiteral> \
    constexpr T(FloatingPointLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_FLOATING_POINT_OPERATORS(T, Underlying) \
    CPRIME_ARITHMETIC_SIGNED_OPERATORS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << self.value; } \
    __VA_ARGS__

#define CPRIME_CHARACTER_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_CONSTRUCTORS(T, Underlying) \
    template <typename CharacterLiteral> requires std::is_integral_v<CharacterLiteral> \
    constexpr T(CharacterLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_BITWISE_OPERATORS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << static_cast<char>(self.value); } \
    __VA_ARGS__

#define CPRIME_BOOLEAN_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_CONSTRUCTORS(T, Underlying) \
    CPRIME_EQUALITY_OPERATORS(T) \
    CPRIME_BOOLEAN_OPERATORS(T, Underlying) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << (self.value ? "true" : "false"); } \
    __VA_ARGS__

// A composite underlying type (`std::array<T, Size>`, ...) has a
// top-level comma, so it is always passed parenthesised and unwrapped here with
// CPRIME_EXPAND rather than routed through CPRIME_COMMON_MEMBERS.
#define CPRIME_COMPOSITE_MEMBERS(T, Underlying, ...) \
    private: \
    CPRIME_EXPAND Underlying value; \
    public: \
    T(const CPRIME_EXPAND Underlying& value_arg) = delete; \
    constexpr T(CPRIME_EXPAND Underlying&& value_arg) : value(std::move(value_arg)) {} \
    template <std::size_t... CPrimeArrayIndices> \
    constexpr T(std::initializer_list<typename decltype(value)::value_type> elements, std::index_sequence<CPrimeArrayIndices...>) \
        : value{*(elements.begin() + CPrimeArrayIndices)...} {} \
    constexpr T(std::initializer_list<typename decltype(value)::value_type> elements) \
        : T(elements, std::make_index_sequence<std::tuple_size_v<std::remove_cv_t<decltype(value)>>>()) {} \
    [[nodiscard]] explicit constexpr operator CPRIME_EXPAND Underlying() const { return value; } \
    [[nodiscard]] constexpr auto operator[](std::size_t index) -> decltype(auto) { return value[index]; } \
    [[nodiscard]] constexpr auto operator[](std::size_t index) const -> decltype(auto) { return value[index]; } \
    CPRIME_EQUALITY_OPERATORS(T) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { \
        stream << '['; \
        for (std::size_t i = 0; i < self.value.size(); ++i) { \
            if (i != 0) { \
                stream << ", "; \
            } \
            stream << self.value[i]; \
        } \
        return stream << ']'; \
    } \
    __VA_ARGS__

#define CPRIME_UNSIGNED_INTEGER_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_UNSIGNED_INTEGER_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_SIGNED_INTEGER_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_SIGNED_INTEGER_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_FLOATING_POINT_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_FLOATING_POINT_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_CHARACTER_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_CHARACTER_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_BOOLEAN_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_BOOLEAN_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_COMPOSITE_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_COMPOSITE_MEMBERS(T, Underlying, __VA_ARGS__) \
    };
// NOLINTEND(bugprone-macro-parentheses)

namespace cprime {
    CPRIME_UNSIGNED_INTEGER_TYPE(uint8, std::uint8_t)
    CPRIME_UNSIGNED_INTEGER_TYPE(uint16, std::uint16_t)
    CPRIME_UNSIGNED_INTEGER_TYPE(uint32, std::uint32_t)
    CPRIME_UNSIGNED_INTEGER_TYPE(uint64, std::uint64_t)

    CPRIME_SIGNED_INTEGER_TYPE(int8, std::int8_t)
    CPRIME_SIGNED_INTEGER_TYPE(int16, std::int16_t)
    CPRIME_SIGNED_INTEGER_TYPE(int32, std::int32_t)
    CPRIME_SIGNED_INTEGER_TYPE(int64, std::int64_t)

    CPRIME_FLOATING_POINT_TYPE(float32, float)
    CPRIME_FLOATING_POINT_TYPE(float64, double)

    CPRIME_CHARACTER_TYPE(char8, std::uint8_t)
    CPRIME_CHARACTER_TYPE(char16, std::uint16_t)
    CPRIME_CHARACTER_TYPE(char32, std::uint32_t)

    template <typename T, std::size_t Size>
    CPRIME_COMPOSITE_TYPE(array, (std::array<T, Size>))
}

#endif  // CPRIME_RUNTIME_HPP
