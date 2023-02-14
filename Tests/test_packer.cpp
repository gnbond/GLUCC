// include the file under test first to prove its #includes are complete
#include "packer.hpp"

// Catch2
#include <catch2/catch_test_macros.hpp>

// System headers
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

using namespace kerry;

using result = std::initializer_list<std::uint8_t>;

TEST_CASE("packer basic", "[packer]") {
    packer p{};

    CHECK(p.size() == 0);
    CHECK(p.target_size() == 0);
    CHECK(p == result{});
}

TEST_CASE("packer basic - size", "[packer]") {
    packer p{10};

    CHECK(p.size() == 0);
    CHECK(p.capacity() >= 10);
    CHECK(p.target_size() == 10);
    CHECK(p == result{});
}

TEST_CASE("packer bytes", "[packer]") {
    packer p{};

    p << std::byte{1} << std::byte{2};
    CHECK(p.size() == 2);
    CHECK(p == result{1, 2});
}

TEST_CASE("packer char types", "[packer]") {
    packer p{};
    char c1 = 'a';
    signed char c2 = 2;
    unsigned char c3 = '\0';

    p << c1 << c2 << c3 << '\x33';
    CHECK(p.size() == 4);
    CHECK(p == result{'a', 2, 0, 0x33});
}

TEST_CASE("packer Bool", "[packer]") {
    packer p{};
    bool b1 = true;
    bool b2 = false;

    p << b1 << b2;
    CHECK(p.size() == 2);
    auto r = result{1, 0};
    CHECK(p == r);
    // Test begin()/end()
    CHECK(std::equal(
        p.begin(), p.end(), r.begin(),
        [](std::byte l, std::uint8_t r) { return l == std::byte{r}; }));
}

TEST_CASE("packer short types", "[packer]") {
    packer p{};
    short s1 = -2;
    unsigned short s2 = 0x3344;

    p << s1 << s2;
    CHECK(p.size() == 4);
    CHECK(p == result{0xff, 0xfe, 0x33, 0x44});
}

TEST_CASE("packer 32bit types", "[packer]") {
    packer p{};
    std::int32_t s1 = -2;
    std::uint32_t s2 = 0x11223344;

    p << s1 << s2;
    CHECK(p.size() == 8);
    CHECK(p == result{0xff, 0xff, 0xff, 0xfe, 0x11, 0x22, 0x33, 0x44});
}

/*

    This should not compile (assuming long is > 32 bits) as the operator<< is
    ambiguous (unit16 and uint32 are equally close matches).  This is
    intentional, there is no htonll() function to fix byteorder for 8-byte
    integers.

    long l1 = 1;
    unsigned long l2 = 2;

    p << l1 << l2;
*/

TEST_CASE("packer byte array", "[packer]") {
    packer p{};

    std::byte arr[] = {std::byte{1}, std::byte{2}, std::byte{3}};
    p << arr;
    CHECK(p.size() == 3);
    CHECK(p == result{1, 2, 3});
}

TEST_CASE("packer char array", "[packer]") {
    packer p{};

    char arr[] = {1, 2, 3};
    p << arr;
    CHECK(p.size() == 3);
    CHECK(p == result{1, 2, 3});
}

TEST_CASE("packer short array", "[packer]") {
    packer p{};

    std::int16_t arr[] = {1, -2};
    p << arr;
    CHECK(p.size() == 4);
    CHECK(p == result{0, 1, 0xff, 0xfe});
}

/*
    This should not compile
    std::uint64_t arr[] = {1, -2};
    p << arr;
  */

TEST_CASE("packer size exception", "[packer]") {
    packer p{6};
    std::int32_t val = 2;
    p << val;
    CHECK_THROWS_AS(p.data(), size_error);
    std::uint16_t val2 = 3;
    p << val2;
    CHECK(p.size() == 6);
    CHECK(p.data());  // Size is correct, this is OK
    p << 'a';
    CHECK(p.size() == 7);
    CHECK(p.size() > p.target_size());
    CHECK_THROWS_AS(p.data(), size_error);
}

struct Data {
    char len{2};
    short values[4] = {3, -2, 0, 0};
    std::uint32_t checksum{0x12345678};
};

packer& operator<<(packer& p, const Data& data) {
    return p << data.len << data.values << data.checksum;
}

TEST_CASE("packer arbitrary struct", "[packer]") {
    packer p{};
    Data d;
    p << d;
    CHECK(p.size() == 13);
    CHECK(p == result{2, 0, 3, 0xff, 0xfe, 0, 0, 0, 0, 0x12, 0x34, 0x56, 0x78});
}
