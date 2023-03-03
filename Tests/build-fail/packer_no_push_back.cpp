#include "packer.hpp"

struct Foo {};

// expect: no matching member function for call to 'push_back'

int main() {
    kerry::packer p{};

    p.push_back(Foo{});
}
