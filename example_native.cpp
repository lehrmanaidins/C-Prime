#include "src/runtime/c-prime.hpp"

struct NativeDistance {
    cprime::uint32 value;
};

constexpr cprime::uint32 native_seed = 11;

cprime::uint32 native_add(cprime::uint32 left, cprime::uint32 right) {
    return left + right;
}
