#include "unpacker.hpp"

// Header first

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <initializer_list>
#include <list>
#include <new>
#include <stdexcept>
#include <vector>

using namespace james;

using bytes = std::initializer_list<unsigned char>;

TEST_CASE("unpacker type traits", "[unpacker]") {
    // Vector of any byte integral is a byte container
    CHECK(details::is_byte_container<std::vector<char>>);
    CHECK(details::is_byte_container<std::vector<unsigned char>>);
    CHECK(details::is_byte_container<std::vector<signed char>>);
    CHECK(details::is_byte_container<std::vector<std::byte>>);

    // Initializer_list is contiguous
    CHECK(details::is_byte_container<std::initializer_list<unsigned char>>);

    // std::array is contiguous
    CHECK(details::is_byte_container<std::array<char, 2>>);

    // vector<bool> is not contiguous, but initializer_list<bool> is
    CHECK(details::is_byte_container<std::initializer_list<bool>>);

    // List is not a byte container (not contiguous)
    CHECK_FALSE(details::is_byte_container<std::list<char>>);

    // ints and pointers are not byte integrals
    CHECK_FALSE(details::is_byte_container<std::vector<int>>);
    CHECK_FALSE(details::is_byte_container<std::vector<void*>>);
}

TEST_CASE("unpacker basic", "[unpacker]") {
    bytes data{1, 2};

    unpacker u{data};
    CHECK(u.size() == 2);
    CHECK(u.remaining() == 2);

    unsigned char c1{};
    unsigned char c2{};
    u >> c1 >> c2;
    CHECK(u.remaining() == 0);
    CHECK(c1 == 1);
    CHECK(c2 == 2);

    u.reset();
    CHECK(u.remaining() == 2);

    unsigned short us{0};
    u >> us;
    CHECK(u.remaining() == 0);
    CHECK(us == 0x0102);
}

TEST_CASE("unpacker basic inline", "[unpacker]") {
    unpacker u{bytes{2, 3, 4}};
    CHECK(u.size() == 3);
    CHECK(u.remaining() == 3);
}

TEST_CASE("unpacker std::array", "[unpacker]") {
    std::array r{'\2', '\3'};
    unpacker u{r};
    CHECK(u.size() == 2);
    CHECK(u.remaining() == 2);
}

TEST_CASE("unpacker C array", "[unpacker]") {
    unsigned char r[] = {2, 3};
    unpacker u{r};
    CHECK(u.size() == 2);
    CHECK(u.remaining() == 2);
}

TEST_CASE("unpacker std::byte", "[unpacker]") {
    std::byte r[] = {std::byte{2}, std::byte{3}};
    unpacker u{&r[0], sizeof(r)};
    CHECK(u.size() == 2);
    CHECK(u.remaining() == 2);
}

TEST_CASE("unpacker overrun", "[unpacker]") {
    bytes r{2, 3};
    unpacker u{r};
    CHECK(u.size() == 2);
    CHECK(u.remaining() == 2);

    std::uint32_t u32{};
    CHECK_THROWS_AS(u >> u32, std::length_error);
}
