#include <array>
#include <cstdint>
#include <initializer_list>

#include "unpacker.hpp"

using namespace james;

// expect:  invalid operands to binary expression

void foo() {
    unpacker u{std::initializer_list<unsigned char>{1, 2}};
    std::uint64_t ul;
    u >> ul;
}
