#ifndef KERRY_PACKER_H
#define KERRY_PACKER_H

/**
 * @file packer.hpp
 * @author Gregory Bond (greg@bond.id.au)
 * @copyright This file is in the public domain.  See <https://unlicense.org>
 *
 * This and other fine code available from https://github.com/gnbond/GLUCC
 *
 * Comments / issues / pull requests gladly accepted
 */

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "is_insertable.hpp"

namespace kerry {

/**
 * @brief Throw this error when added data exceeds pre-defined size
 *
 */
struct size_error : public std::length_error {
    using std::length_error::length_error;
};

/**
 @brief Pack data into a network packet in a C++ manner

 Dealing with binary protocol packets hasn't really changed in 40 years - it's
 still usually C code, and usually a horrible mix of type punning, unsafe
 pointer casts, and calls to `htonl()` and `memcpy()`, all written with silent
 prayers that the new 64-bit compiler version won't add invisible padding bytes
 to your structs.

 Then you run a static analyzer over your code, and have to silence hundreds of
 warnings about all those unsafe 40-year-old coding practices.

There's gotta be a better way.  A C++ way.

Well, now there is.

kerry::packer uses an API inspired by C++ iostreams inserters that is extensible
and type-safe.  It wraps `std::vector<std::byte>` to store the generated packet
data.  It provides inserters for 1/2/4-byte integral types (converting to
network byte order as required).  It provides inserters for fixed-size (C-style)
arrays of supported types, including arrays of custom types with user-written
inserters.

The packer can be created in two modes.  The default-constructed packer is
suitable for variable-sized network packets, leaving the calling code to be
responsible for checking the size of the generated packet matches expectation,
if that is required.  A packer created with a size argument is suitable for
fixed-size network packets, and the packer will check that the generated packet
is exactly that many bytes (by throwing an exception when the data is extracted
if the size is not exactly correct).

Another annoyance is the inconsistencies with the types used to store binary
packets and the standard functions used to manipulate those packets.  The packer
uses std::byte as the native type (this use-case seems to be the main intention
of the std::byte type) but allows implicit conversions to `void*`, `char*` and
`unsigned char*`.  So the following code is typical:
```
packer p;
int sock = socket(...);
//...
p << some << data;
send(sock, p, p.size(), 0)
```

A somewhat contrived example:
```
struct item { uint8_t tag; uint16_t value; }
struct item_packet { uint8_t count; item items[4]; uint32_t checksum; }

packer& operator<<(packer& p, const item& i) {
    return p << i.count << i.value;
}
packer& operator<<(packer& p, const item_packet& ip) {
    return p << ip.count << ip.items << ip.checksum;
}

// ...

send(int sock, const item_packet& ip) {
    packer p{17};
    p << ip;
    if (send(sock, p, p.size()) < 0) {
        // handle error
    }
}
```
What's with the namespace?  This is a little pun that should raise a smile from
anyone familiar with the last 40 years of Australian media or political affairs.
Or the history of cricket in the late 20th century.

TBD:
- member documentation
- example usage
- consider binary blobs.  make copy() public?
- support other containers of packable types? (std::array, std::vector?
iterators? )
 */
class packer {
   public:
    /**
     * @brief Construct a new packer object with variable size
     *
     */
    packer() = default;

    /**
     * @brief Construct a new packer object with a known packet size
     *
     * data() will throw size_error if there are not exactly this many bytes of
     * data
     *
     * @param size packet size
     */
    packer(std::size_t size) : m_target_size{size} { m_data.reserve(size); }

    /**
     * @brief Reserve space for size bytes
     *
     * This is a no-op if the packer was created with a target size
     *
     * @param size
     */
    void reserve(std::size_t size) {
        if (!m_target_size) m_data.reserve(size);
    }

    /**
     * @brief Return the cuurent size of the packed data, in bytes
     *
     * @return auto
     */
    [[nodiscard]] auto size() const { return m_data.size(); }
    /**
     * @brief Return a pointer to the constructed packet
     *
     * This will throw size_error if the packer was created with a target size
     * and there are not exactly that many bytes in the packet.
     *
     * @return const std::byte*
     */
    [[nodiscard]] auto data() const {
        check_size();
        return m_data.data();
    }
    [[nodiscard]] operator const void*() { return data(); }
    [[nodiscard]] operator const std::byte*() { return data(); }
    [[nodiscard]] operator const char*() {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const char*>(data());
    }
    [[nodiscard]] operator const unsigned char*() {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const unsigned char*>(data());
    }
    [[nodiscard]] auto capacity() const { return m_data.capacity(); }
    [[nodiscard]] auto target_size() const { return m_target_size; }
    [[nodiscard]] auto begin() const { return m_data.begin(); }
    [[nodiscard]] auto end() const { return m_data.end(); }

    void clear() { m_data.clear(); }

    template <typename C>
    bool operator==(const C& rhs) {
        return size() == rhs.size() &&
               std::equal(begin(), end(), rhs.begin(),
                          [](std::byte l, typename C::value_type r) {
                              return l == std::byte{r};
                          });
    }

    packer& operator<<(std::byte val) {
        m_data.push_back(val);
        return *this;
    }

    // This template works for all integral 1-byte types, including bool
    // but does not allow larger integral types to accidentally use it
    template <typename B>
    std::enable_if_t<std::is_integral_v<B> && sizeof(B) == 1, packer&>
    operator<<(B b) {
        copy(&b, 1);
        return *this;
    }

    template <std::size_t N>
    packer& operator<<(const std::byte (&b)[N]) {
        copy(b, N);
        return *this;
    }

    /**
     * @brief Efficiently insert a C-style array of 1-byte integral types
     *
     * @tparam B a 1-byte integral type
     * @tparam N Size of the array
     * @return packer&
     */
    template <typename B, std::size_t N>
    std::enable_if_t<std::is_integral_v<B> && sizeof(B) == 1, packer&>
    operator<<(const B (&b)[N]) {
        copy(&b, N);
        return *this;
    }

    // Allow C-style arrays of any non-byte-sized type for which we already have
    // an inserter.  This uses the details: so has to be a free function
    // template, not a member. C-style arrays of byte-sized types are handled in
    // the class definition above using a direct call to copy() for efficiency
    template <typename T, std::size_t N>
    std::enable_if_t<sizeof(T) >= 2 && glucc::is_insertable_into_v<T, packer>,
                     packer&>
    operator<<(const T (&b)[N]) {
        const T* p = b;
        for (int n = N; n > 0; --n, ++p) {
            *this << *p;
        }
        return *this;
    }

    packer& operator<<(std::int16_t val) {
        val = htons(val);
        copy(&val, 2);
        return *this;
    }
    packer& operator<<(std::uint16_t val) {
        val = htons(val);
        copy(&val, 2);
        return *this;
    }

    packer& operator<<(std::int32_t val) {
        val = htonl(val);
        copy(&val, 4);
        return *this;
    }

    packer& operator<<(std::uint32_t val) {
        val = htonl(val);
        copy(&val, 4);
        return *this;
    }

   private:
    std::vector<std::byte> m_data{};
    std::size_t m_target_size{0};

    void copy(const void* p, int n) {
        // half the point of this class is to hide the inevitable casts and
        // pointer messes
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto bp = reinterpret_cast<const std::byte*>(p);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        m_data.insert(m_data.end(), bp, bp + n);
    }

    void check_size() const {
        if (m_target_size > 0 && size() != m_target_size) {
            throw size_error{std::string("packer size ") +
                             std::to_string(size()) + " <> " +
                             std::to_string(m_target_size)};
        }
    }
};

}  // namespace kerry

#endif  // KERRY_PACKER_H
