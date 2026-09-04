
#ifndef CPRIME_RUNTIME_HPP
#define CPRIME_RUNTIME_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdfloat>
#include <iostream>
#include <tuple>
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

#define CPRIME_COMMON_MEMBERS(T, Underlying) \
    private: \
    Underlying value; \
    public: \
    T(const Underlying& value_arg) = delete; \
    constexpr T(Underlying&& value_arg) : value(std::forward<Underlying>(value_arg)) {} \
    [[nodiscard]] explicit constexpr operator Underlying() const { return value; } \

#define CPRIME_LITERAL_OPERATORS(T, Underlying) \
    template <typename Literal> requires std::is_arithmetic_v<Literal> \
    constexpr T(Literal literal) : value(static_cast<Underlying>(std::move(literal))) {}

#define CPRIME_EQUALITY_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator==(T other) const -> bool { return value == other.value; } \
    [[nodiscard]] constexpr auto operator!=(T other) const -> bool { return value != other.value; }

#define CPRIME_ORDERED_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator<(T other) const -> bool { return value < other.value; } \
    [[nodiscard]] constexpr auto operator<=(T other) const -> bool { return value <= other.value; } \
    [[nodiscard]] constexpr auto operator>(T other) const -> bool { return value > other.value; } \
    [[nodiscard]] constexpr auto operator>=(T other) const -> bool { return value >= other.value; }

#define CPRIME_COMPARISON_OPERATORS(T, Underlying) \
    CPRIME_EQUALITY_OPERATORS(T, Underlying) \
    CPRIME_ORDERED_OPERATORS(T, Underlying)

#define CPRIME_ARITHMETIC_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator+(T other) const -> T { return T(static_cast<Underlying>(value + other.value)); } \
    [[nodiscard]] constexpr auto operator-(T other) const -> T { return T(static_cast<Underlying>(value - other.value)); } \
    [[nodiscard]] constexpr auto operator*(T other) const -> T { return T(static_cast<Underlying>(value * other.value)); } \
    [[nodiscard]] constexpr auto operator/(T other) const -> T { return T(static_cast<Underlying>(value / other.value)); } \
    constexpr auto operator+=(T other) -> T& { value = static_cast<Underlying>(value + other.value); return *this; } \
    constexpr auto operator-=(T other) -> T& { value = static_cast<Underlying>(value - other.value); return *this; } \
    constexpr auto operator*=(T other) -> T& { value = static_cast<Underlying>(value * other.value); return *this; } \
    constexpr auto operator/=(T other) -> T& { value = static_cast<Underlying>(value / other.value); return *this; }

#define CPRIME_MODULUS_OPERATORS(T, Underlying) \
    [[nodiscard]] constexpr auto operator%(T other) const -> T { return T(static_cast<Underlying>(value % other.value)); } \
    constexpr auto operator%=(T other) -> T& { value = static_cast<Underlying>(value % other.value); return *this; }

#define CPRIME_SIGNED_MEMBERS(T, Underlying) \
    [[nodiscard]] constexpr auto operator+() const -> T { return *this; } \
    [[nodiscard]] constexpr auto operator-() const -> T { return T(static_cast<Underlying>(-value)); }

#define CPRIME_BITWISE_MEMBERS(T, Underlying) \
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

// The *_MEMBERS macros are struct-body fragments: operators plus any extra
// member functions passed as trailing arguments. They are used by the *_TYPE
// macros below and are also emitted directly into a hand-written wrapper struct
// for C-Prime `type` definitions, so that struct's constructor can carry
// contracts and its `<<` can delegate to the wrapped value's own streaming.
#define CPRIME_UNSIGNED_INTEGER_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_MEMBERS(T, Underlying) \
    template <typename UnsignedIntegerLiteral> requires std::is_integral_v<UnsignedIntegerLiteral> \
    constexpr T(UnsignedIntegerLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_OPERATORS(T, Underlying) \
    CPRIME_MODULUS_OPERATORS(T, Underlying) \
    CPRIME_BITWISE_MEMBERS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T, Underlying) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << cprimeStreamable(self.value); } \
    __VA_ARGS__

#define CPRIME_SIGNED_INTEGER_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_MEMBERS(T, Underlying) \
    template <typename SignedIntegerLiteral> requires std::is_integral_v<SignedIntegerLiteral> \
    constexpr T(SignedIntegerLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_OPERATORS(T, Underlying) \
    CPRIME_MODULUS_OPERATORS(T, Underlying) \
    CPRIME_SIGNED_MEMBERS(T, Underlying) \
    CPRIME_BITWISE_MEMBERS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T, Underlying) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << cprimeStreamable(self.value); } \
    __VA_ARGS__

#define CPRIME_FLOAT_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_MEMBERS(T, Underlying) \
    template <typename FloatingPointLiteral> requires std::is_floating_point_v<FloatingPointLiteral> \
    constexpr T(FloatingPointLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_OPERATORS(T, Underlying) \
    CPRIME_SIGNED_MEMBERS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T, Underlying) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << self.value; } \
    __VA_ARGS__

#define CPRIME_CHARACTER_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_MEMBERS(T, Underlying) \
    template <typename CharacterLiteral> requires std::is_integral_v<CharacterLiteral> \
    constexpr T(CharacterLiteral literal) : value(static_cast<Underlying>(std::move(literal))) {} \
    CPRIME_ARITHMETIC_OPERATORS(T, Underlying) \
    CPRIME_BITWISE_MEMBERS(T, Underlying) \
    CPRIME_COMPARISON_OPERATORS(T, Underlying) \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << static_cast<char>(self.value); } \
    __VA_ARGS__

#define CPRIME_BOOLEAN_MEMBERS(T, Underlying, ...) \
    CPRIME_COMMON_MEMBERS(T, Underlying) \
    CPRIME_EQUALITY_OPERATORS(T, Underlying) \
    CPRIME_BOOLEAN_MEMBERS(T, Underlying) \
    [[nodiscard]] constexpr auto operator&&(T other) const -> T { return T(value && other.value); } \
    [[nodiscard]] constexpr auto operator||(T other) const -> T { return T(value || other.value); } \
    [[nodiscard]] constexpr auto operator!() const -> T { return T(!value); } \
    friend auto operator<<(std::ostream& stream, const T& self) -> std::ostream& { return stream << (self.value ? "true" : "false"); } \
    __VA_ARGS__

// A composite underlying type (`std::array<T, Size>`, a tuple, ...) has a
// top-level comma, so it is always passed parenthesised and unwrapped here with
// CPRIME_EXPAND rather than routed through CPRIME_COMMON_MEMBERS.
#define CPRIME_COMPOSITE_MEMBERS(T, Underlying, ...) \
    private: \
    CPRIME_EXPAND Underlying value; \
    public: \
    T(const CPRIME_EXPAND Underlying& value_arg) = delete; \
    constexpr T(CPRIME_EXPAND Underlying&& value_arg) : value(std::move(value_arg)) {} \
    [[nodiscard]] explicit constexpr operator CPRIME_EXPAND Underlying() const { return value; } \
    [[nodiscard]] constexpr auto operator[](std::size_t index) -> decltype(value[index]) { return value[index]; } \
    [[nodiscard]] constexpr auto operator[](std::size_t index) const -> decltype(value[index])& { return value[index]; } \
    [[nodiscard]] constexpr auto operator==(const T& other) const -> bool { return value == other.value; } \
    [[nodiscard]] constexpr auto operator!=(const T& other) const -> bool { return value != other.value; } \
    __VA_ARGS__

#define CPRIME_UNSIGNED_INTEGER_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_UNSIGNED_INTEGER_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_SIGNED_INTEGER_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_SIGNED_INTEGER_MEMBERS(T, Underlying, __VA_ARGS__) \
    };

#define CPRIME_FLOAT_TYPE(T, Underlying, ...) \
    struct T { \
        CPRIME_FLOAT_MEMBERS(T, Underlying, __VA_ARGS__) \
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

namespace cprime {
    CPRIME_UNSIGNED_INTEGER_TYPE(uint8, std::uint8_t)
    CPRIME_UNSIGNED_INTEGER_TYPE(uint16, std::uint16_t)
    CPRIME_UNSIGNED_INTEGER_TYPE(uint32, std::uint32_t)
    CPRIME_UNSIGNED_INTEGER_TYPE(uint64, std::uint64_t)

    CPRIME_SIGNED_INTEGER_TYPE(int8, std::int8_t)
    CPRIME_SIGNED_INTEGER_TYPE(int16, std::int16_t)
    CPRIME_SIGNED_INTEGER_TYPE(int32, std::int32_t)
    CPRIME_SIGNED_INTEGER_TYPE(int64, std::int64_t)

    CPRIME_FLOAT_TYPE(float32, float)
    CPRIME_FLOAT_TYPE(float64, double)

    CPRIME_CHARACTER_TYPE(char8, std::uint8_t)
    CPRIME_CHARACTER_TYPE(char16, std::uint16_t)
    CPRIME_CHARACTER_TYPE(char32, std::uint32_t)

    template <typename T, std::size_t Size>
    CPRIME_COMPOSITE_TYPE(array, (std::array<T, Size>))
}

#endif  // CPRIME_RUNTIME_HPP
