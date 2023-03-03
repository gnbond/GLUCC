#include "packer.hpp"

// expect: use of overloaded operator '<<' is ambiguous

int main() {
    kerry::packer p{};

    p << 1L;
}
