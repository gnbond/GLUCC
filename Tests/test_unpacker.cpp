#include "unpacker.hpp"

// Catch first
#include <catch2/catch_test_macros.hpp>

// Then system headers

#include <array>
#include <cstdint>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <vector>

using namespace james;

using bytes = std::initializer_list<unsigned char>;

struct Extractable {};
unpacker& operator>>(unpacker& u, Extractable& e);

struct NotExtractable {};

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
    CHECK_FALSE(details::is_byte_container<std::vector<bool>>);
    CHECK(details::is_byte_container<std::initializer_list<bool>>);

    // List is not a byte container (not contiguous)
    CHECK_FALSE(details::is_byte_container<std::list<char>>);

    // ints and pointers are not byte integrals
    CHECK_FALSE(details::is_byte_container<std::vector<int>>);
    CHECK_FALSE(details::is_byte_container<std::vector<void*>>);
}

TEST_CASE("unpacker extractable type traits", "[unpacker]") {
    CHECK(details::unpackable_v<char>);
    CHECK(details::unpackable_v<unsigned char>);
    CHECK(details::unpackable_v<signed char>);
    CHECK(details::unpackable_v<std::byte>);
    CHECK(details::unpackable_v<bool>);

    CHECK(details::unpackable_v<std::uint16_t>);
    CHECK(details::unpackable_v<std::int16_t>);
    CHECK(details::unpackable_v<std::uint32_t>);
    CHECK(details::unpackable_v<std::int32_t>);

    CHECK(details::unpackable_v<Extractable>);

    CHECK_FALSE(details::unpackable_v<NotExtractable>);
    CHECK_FALSE(details::unpackable_v<char*>);
    CHECK_FALSE(details::unpackable_v<double>);
    CHECK_FALSE(details::unpackable_v<std::uint64_t>);
    CHECK_FALSE(details::unpackable_v<std::vector<int>>);
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

TEST_CASE("unpacker to bool", "[unpacker]") {
    bytes data{2, 0};
    bool b1{};
    bool b2{};
    unpacker u{data};
    u >> b1 >> b2;
    CHECK(u.remaining() == 0);
    CHECK(b1);
    CHECK_FALSE(b2);
}
TEST_CASE("unpacker overrun", "[unpacker]") {
    bytes r{2, 3, 4};
    unpacker u{r};
    CHECK(u.size() == 3);
    CHECK(u.remaining() == 3);

    std::uint32_t u32{};
    CHECK_THROWS_AS(u >> u32, std::length_error);
}

TEST_CASE("unpacker array extractor", "[unpacker]") {
    unpacker u{bytes{1, 2, 3, 4}};
    CHECK(u.size() == 4);
    CHECK(u.remaining() == 4);
    std::uint16_t a[2] = {};
    u >> a;
    CHECK(u.remaining() == 0);
    CHECK(a[0] == 0x0102);
    CHECK(a[1] == 0x0304);
}

TEST_CASE("unpacker std::array extractor", "[unpacker]") {
    unpacker u{bytes{1, 2, 3, 4}};
    CHECK(u.size() == 4);
    CHECK(u.remaining() == 4);
    std::array<std::uint16_t, 2> a{};
    u >> a;
    CHECK(u.remaining() == 0);
    CHECK(a[0] == 0x0102);
    CHECK(a[1] == 0x0304);
}
