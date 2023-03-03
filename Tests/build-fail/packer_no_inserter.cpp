#include "packer.hpp"

struct Foo {};

// expect: invalid operands to binary expression ('kerry::packer' and 'Foo')

int main() {
    kerry::packer p{};

    p << Foo{};
}
