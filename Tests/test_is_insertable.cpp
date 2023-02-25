#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "is_insertable.hpp"

using namespace glucc;

struct Foo {};

TEST_CASE("insertable", "[is_insertable]") {
    CHECK(is_insertable_into_v<int, std::ostream>);
    // this should pass, as ostringstream << int returns ref to a base class of
    // ostringstream
    CHECK(is_insertable_into_v<int, std::ostringstream>);

    CHECK_FALSE(is_insertable_into_v<Foo, std::ostream>);
    // 1 << 4 is a well-defined expression, but that does not represnt insertion
    // into a stream
    CHECK_FALSE(is_insertable_into_v<int, int>);
}
