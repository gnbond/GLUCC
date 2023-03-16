#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <utility>

#include "is_insertable.hpp"

using namespace glucc;

// A test type with no defined operations
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

// Checks for custom streams, with both member function and free function
// inserters
struct Baz {};
struct Stream {
    Stream& operator<<(const Baz&);
};
struct Bar {};
Stream& operator<<(Stream&, const Bar&);

TEST_CASE("custom stream", "[is_insertable]") {
    CHECK(is_insertable_into_v<Bar, Stream>);
    CHECK(is_insertable_into_v<Baz, Stream>);
    CHECK_FALSE(is_insertable_into_v<int, Stream>);
    CHECK_FALSE(is_insertable_into_v<Foo, Stream>);
}
