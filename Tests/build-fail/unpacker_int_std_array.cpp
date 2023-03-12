#include <array>

#include "unpacker.hpp"

using namespace james;

// expect: requirement 'is_byte_container

// std::array<int, 2>
const std::array data{1, 2};

const unpacker u{data};
