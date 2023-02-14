#ifndef KERRY_PACKER_H
#define KERRY_PACKER_H

/**
 * @file packer.h
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

 Dealing with binary network packets hasn't really changed in 40 years - it's
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
arrays of integral types.

The packer can be created in two modes.  The default-constructed packer is
suitable for variable-sized network packets, leaving the calling code to be
responsible for checking the size of the generated packet matches expectation,
if that is required.  A packer created with a size argument is suitable for
fixed-size network packets, and the packer will check that the generated packet
is exactly that many bytes (by throwing an exception when the data is extracted
if the size is not exactly correct).

What's with the namespace?  This is a little pun that should raise a smile from
anyone familiar with the last 40 years of Australian media or political affairs.
Or the history of cricket in the late 20th century.

TBD: member documentation
TBD: example usage
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

    [[nodiscard]] auto size() const { return m_data.size(); }
    [[nodiscard]] auto data() const {
        check_size();
        return m_data.data();
    }
    [[nodiscard]] auto capacity() const { return m_data.capacity(); }
    [[nodiscard]] auto target_size() const { return m_target_size; }
    [[nodiscard]] auto begin() const { return m_data.begin(); }
    [[nodiscard]] auto end() const { return m_data.end(); }

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

    template <typename B, std::size_t N>
    std::enable_if_t<std::is_integral_v<B> && sizeof(B) == 1, packer&>
    operator<<(const B (&b)[N]) {
        copy(&b, N);
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

// Hide some template meta-hackery here
// This has to be after definition of packer because we are accessing
// packer::operator<<()

namespace details {

// True if T has a defined inserter for packer
template <typename T>
constexpr bool is_insertable_v =
    std::is_reference_v<decltype(std::declval<packer>().operator<<(
        std::declval<T>()))>;

// True if T has size > 1 and a defined inserter for packer
template <typename T>
constexpr bool is_nonbyte_insertable_v = sizeof(T) > 1 && is_insertable_v<T>;

}  // namespace details

// Allow C-style arrays of any non-byte-sized type for which we already have an
// inserter.  This uses the details: so has to be a free function template, not
// a member. C-style arrays of byte-sized types are handled in the class
// definition above using a direct call to copy() for efficiency
template <typename B, std::size_t N>
std::enable_if_t<details::is_nonbyte_insertable_v<B>, packer&> operator<<(
    packer& out, const B (&b)[N]) {
    const B* p = b;
    for (int n = N; n > 0; --n, ++p) out << *p;
    return out;
}

}  // namespace kerry

#endif  // KERRY_PACKER_H
