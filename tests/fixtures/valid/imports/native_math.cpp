#include "src/runtime/c-prime.hpp"

struct NativeDistance {
    cprime::primitive::uint32 value;
};

constexpr cprime::primitive::uint32 native_seed = 11;

cprime::primitive::uint32 native_add(cprime::primitive::uint32 left, cprime::primitive::uint32 right) {
    return left + right;
}