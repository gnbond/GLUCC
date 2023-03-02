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

/// Forward-declare this so we can define the type trait
class packer;

namespace details {
/**
 * @brief A type trait to represent a type that can be inserted into a packer
 *
 * @tparam T
 */
template <typename T>
constexpr bool packable = glucc::is_insertable_into_v<T, packer>;

/**
 * @brief A type trait to represent a trivial type that can be inserted into
 * a packer
 *
 * @tparam T
 */
template <typename T>
constexpr bool byte_packable = sizeof(T) == 1 && packable<T>;

/**
 * @brief A type trait to represent a non-trivial type that can be inserted into
 * a packer
 *
 * @tparam T
 */
template <typename T>
constexpr bool multibyte_packable = sizeof(T) >= 2 && packable<T>;

/**
 * @brief A type trait that is true if T is a 1-byte integral type
 *
 * @tparam T
 */
template <typename T>
constexpr bool byte_integral = sizeof(T) == 1 && std::is_integral_v<T>;

}  // namespace details

/**
 @brief Pack data into a binary protocol packet in a C++ manner

 Dealing with binary protocol packets hasn't really changed in 40 years - it's
 still usually C code, and usually a horrible mix of type punning, malloc()'d
 memory, unsafe pointer casts, and calls to `htonl()` and `memcpy()`, all
 written with silent prayers that the new 64-bit compiler version won't add
 invisible padding bytes to your structs.

 Then you run a static analyzer over your code, and have to silence hundreds of
 warnings about all those unsafe 40-year-old coding practices.  And heaven help
 you if you also need to meet MISRA or similar high-integrity software
 standards.

There's gotta be a better way.  A C++ way.

Well, now there is.

kerry::packer uses an API inspired by C++ iostreams inserters that is
extensible, type-safe and bounds-safe.  It wraps `std::vector<std::byte>` to
store the generated packet data and avoid any memory leaks or bounds errors.  It
provides inserters for 1/2/4-byte integral types (converting to network byte
order as required).  It provides inserters for fixed-size (C-style) arrays and
std::arrays of supported types, including arrays of custom types with
user-written inserters.  It hides all the network byte order conversions, the
reinterpret_cast<>s and the memory copies needed to take a C++ object and turn
it into a byte stream.,

The packer can be created in two modes.  The default-constructed packer is
suitable for variable-sized network packets, leaving the calling code to be
responsible for checking the size of the generated packet matches expectation,
if that is required.  A packer created with a size argument is suitable for
fixed-size network packets, and the packer will check that the generated packet
is exactly that many bytes (by throwing an exception when the data is extracted
if the size is not exactly correct).

Another annoyance in network programming is the inconsistencies with the types
used to store binary packets and the standard functions used to manipulate those
packets.  Old code uses `char *` or sometimes `unsigned char*`.  Slightly less
old code uses `void *`. The packer uses std::byte as the native type (this
use-case seems to be the main intention of the std::byte type) but allows
implicit conversions to `void*`, `char*` and `unsigned char*`.  So the following
code is typical:
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

pack_and_send(int sock, const item_packet& ip) {
    packer p{17};
    p << ip;
    if (send(sock, p, p.size(), 0) < 0) {
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
- support other containers of packable types? (std::vector? iterators? )
 */
class packer {
    using Vector = std::vector<std::byte>;

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
     * @name Standard container member types
     *
     */
    ///@{
    using value_type = Vector::value_type;
    using pointer = Vector::const_pointer;
    using const_pointer = pointer;
    using reference = Vector::const_reference;
    using const_reference = reference;
    ///@}

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
     * @brief Return the current size of the packed data, in bytes
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
     * Beware dangling pointers if the packer is destroyed or if more data is
     * added.
     *
     * @return const std::byte*
     */
    [[nodiscard]] auto data() const {
        check_size();
        return m_data.data();
    }
    /**
     * @name Packet data access
     *
     * Packets can be treated as streams of `std::byte`, `char`, or `unsigned
     * char` types. Packets can be also be accessed as a `void*`.
     *
     * These functions allow implicit conversion to the required type.  Like the
     * `data()` member, they will throw `size_error` for packers constructed
     * with a target size, if the packed data is not exactly the right length.
     *
     * Beware dangling pointers if the packer is destroyed or if more data is
     * added.
     *
     * Typical use might be:
     * ```
     * int send(int sock, void* data, size_t len);
     * //...
     * packer p{20};
     * // Insert exactly 20 bytes of data into p
     * if (send(sock, p, p.size()) < 0) { handle error; }
     * ```
     */
    ///@{
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
    ///@}
    [[nodiscard]] auto capacity() const { return m_data.capacity(); }
    /**
     * @brief The target size for this packer
     *
     * Returns 0 if this packer was not created with a target size
     *
     * @return auto
     */
    [[nodiscard]] auto target_size() const { return m_target_size; }

    /**
     * @name Iterators
     *
     * Iterators are provided to access the data.  Updating packet contents via
     * iterators is not supported.
     */
    ///@{
    [[nodiscard]] auto begin() const { return m_data.begin(); }
    [[nodiscard]] auto end() const { return m_data.end(); }
    ///@}

    /**
     * @brief Clear the packer for re-use
     *
     * Target size is not updated but size() is set to zero and existing packet
     * data is lost.
     */
    void clear() { m_data.clear(); }

    packer& operator<<(std::byte val) {
        m_data.push_back(val);
        return *this;
    }

    /**
     * @brief Insert a 1-byte integral type
     *
     * This template works for all integral 1-byte types, including bool
     * but does not allow larger integral types to accidentally use it
     *
     * 1-byte values do not need network byte order conversion
     *
     * @tparam B A 1-byte integral type (char, unsigned char, signed char, bool)
     * @param b The value of type B to insert
     * @return packer&
     */
    template <typename B>
    std::enable_if_t<details::byte_integral<B>, packer&> operator<<(B b) {
        copy(&b, 1);
        return *this;
    }

    /**
     * @brief Efficiently insert a C-style array of std::byte
     *
     * @tparam N Size of the array
     * @return packer&
     */
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
    std::enable_if_t<details::byte_integral<B>, packer&> operator<<(
        const B (&b)[N]) {
        copy(&b, N);
        return *this;
    }

    /**
     * @brief Insert a fixed-size C-style array of packable types
     *
     * Allow C-style arrays of any non-byte-sized type for which we already have
     * an inserter, including custom types with custom inserters. C-style arrays
     * of byte-sized types integral are handled above using a direct call to
     * copy() for efficiency
     *
     * @tparam T The type of the objects to be inserted
     * @tparam N The number of objects to insert
     * @return packer&
     */
    template <typename T, std::size_t N>
    std::enable_if_t<details::multibyte_packable<T>, packer&> operator<<(
        const T (&b)[N]) {
        const T* p = b;
        for (int n = N; n > 0; --n, ++p) {
            *this << *p;
        }
        return *this;
    }
    /**
     * @brief Inserters for 2 and 4-byte integral types.
     * @name Integral Types
     *
     * These are converted to network byte order.
     *
     */
    ///@{
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
    ///@}

    /**
     * @brief Push any insertable type to the end of the packet
     *
     * Alas, back_insert_iterator<> can only insert Container::value_type (aka
     * std::byte) so this is not as useful as it might be
     *
     * @tparam T an insertable type
     * @param val Value to push into the packet
     */
    template <typename T>
    std::enable_if_t<details::packable<T>> push_back(const T& val) {
        *this << val;
    }

    /**
     * @brief Pack a std::array of single-byte packable data
     *
     * This uses copy() to pack the array efficiently
     *
     * @tparam T The type to pack
     * @tparam N The length of the array
     * @param a
     * @return packer&
     */
    template <typename T, std::size_t N>
    std::enable_if_t<details::byte_packable<T>, packer&> operator<<(
        const std::array<T, N>& a) {
        copy(a.data(), a.size());
        return *this;
    }

    /**
     * @brief Pack a std::array of multi-byte packable data
     *
     * @tparam T The type to pack
     * @tparam N The length of the array
     * @param a
     * @return packer&
     */
    template <typename T, std::size_t N>
    std::enable_if_t<details::multibyte_packable<T>, packer&> operator<<(
        const std::array<T, N>& a) {
        for (auto& t : a) *this << t;
        return *this;
    }

   private:
    Vector m_data{};
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
