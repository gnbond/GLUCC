// include the file under test first to prove its #includes are complete
#include "packer.hpp"

// Catch2
#include <catch2/catch_test_macros.hpp>

// System headers
#include <array>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>

using namespace kerry;

using result = std::initializer_list<std::uint8_t>;

namespace kerry {

// This has to be in namespace kerry:: and be discovered using ADL else the
// Catch2 expression types get confused
bool operator==(const packer& p, const result& r) {
    return p.size() == r.size() && std::equal(p.begin(), p.end(), r.begin(),
                                              [](std::byte l, std::uint8_t r) {
                                                  return l == std::byte{r};
                                              });
}
}  // namespace kerry

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

TEST_CASE("packer short std::array", "[packer]") {
    packer p{};

    std::array<std::int16_t, 2> arr = {1, -2};
    p << arr;
    CHECK(p.size() == 4);
    CHECK(p == result{0, 1, 0xff, 0xfe});
}

TEST_CASE("packer char std::array", "[packer]") {
    packer p{};

    std::array<char, 2> arr = {1, -2};
    p << arr;
    CHECK(p.size() == 2);
    CHECK(p == result{1, 0xfe});
}

TEST_CASE("packer char literal", "[packer]") {
    packer p{};

    p << "Hello";
    CHECK(p.size() == 6);  // Dont forget the terminsating NUL
    CHECK(p == result{'H', 'e', 'l', 'l', 'o', 0});
}

TEST_CASE("packer short push_back", "[packer]") {
    packer p{};

    std::int16_t arr[] = {1, -2};
    p.push_back(arr[0]);
    p.push_back(arr[1]);
    CHECK(p.size() == 4);
    CHECK(p == result{0, 1, 0xff, 0xfe});
}

TEST_CASE("packer data() converters", "[packer]") {
    packer p{};
    const std::byte* bytep = p;
    const char* cp = p;
    const unsigned char* ucp = p;
    const void* voidp = p;
}

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

// This one is from the documentation
struct item {
    uint8_t tag;
    uint16_t value;
};
struct item_packet {
    uint8_t count;
    item items[4];
    uint32_t checksum;
};

packer& operator<<(packer& p, const item& i) { return p << i.tag << i.value; }
packer& operator<<(packer& p, const item_packet& ip) {
    return p << ip.count << ip.items << ip.checksum;
}

TEST_CASE("packer arbitrary struct", "[packer]") {
    packer p{};
    item_packet d = {2, {{3, 5}, {4, 6}}, 0x33445566};
    p << d;
    CHECK(p.size() == 17);
    CHECK(p == result{2, 3, 0, 5, 4, 0, 6, 0, 0, 0, 0, 0, 0, 0x33, 0x44, 0x55,
                      0x66});
}
