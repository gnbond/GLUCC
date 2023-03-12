#include <array>

#include "unpacker.hpp"

using namespace james;

// expect:  no known conversion from 'const int *' to

const int data[] = {1, 2};

const unpacker u{data};
