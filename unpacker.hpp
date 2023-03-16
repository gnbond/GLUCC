#ifndef JAMES_UNPACKER_H
#define JAMES_UNPACKER_H

/**
 * @file unpacker.hpp
 * @author Gregory Bond (greg@bond.id.au)
 * @copyright This file is in the public domain.  See <https://unlicense.org>
 *
 * This and other fine code available from https://github.com/gnbond/GLUCC
 *
 * Comments / issues / pull requests gladly accepted
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace james {

class unpacker;

namespace details {

// Some helper type traits
// A byte container is a ContiguousContainer of 1-byte integral (or std::byte)
// types.  These are containers that are acceptable as packet data to the
// unpacker

// Most types are not byte containers
template <typename C, typename = void>
struct byte_container_helper : std::false_type {};

// Is a byte container if std::data(), std::size() work, if the value_type is
// 1-byte integral or std::byte
template <typename C>
struct byte_container_helper<
    C, std::void_t<
           std::enable_if_t<std::is_integral_v<typename C::value_type> ||
                            std::is_same_v<typename C::value_type, std::byte>>,
           std::enable_if_t<sizeof(typename C::value_type) == 1>,
           decltype(std::data(std::declval<C>())),
           decltype(std::size(std::declval<C>()))>> : std::true_type {};

template <typename C>
constexpr bool is_byte_container = byte_container_helper<C>::value;
template <typename C>
using byte_container_t = std::enable_if_t<is_byte_container<C>, C>;

// A type trait to determine if a type can be extracted from an unpacker
template <typename T, typename V = void>
struct unpackable : std::false_type {};

template <typename T>
struct unpackable<
    T, std::void_t<decltype(std::declval<unpacker&>() >> std::declval<T&>())>>
    : std::true_type {};

template <typename T>
constexpr bool unpackable_v = unpackable<T>::value;

template <typename T>
using unpackable_t = std::enable_if_t<unpackable_v<T>, unpacker&>;

}  // namespace details

class unpacker {
    using iterator = const unsigned char*;
    // delegated constructor with void*, this is private.  Order of arguments is
    // reversed to avoid ambiguous constructors. This hides all the casts in
    // this one spot
    unpacker(std::size_t n, const void* b)
        : m_begin{static_cast<iterator>(b)},
          m_end{m_begin + n},
          m_next{m_begin} {}

   public:
    // unsigned char is the default packet data type
    unpacker(const unsigned char* data, std::size_t size)
        : unpacker(size, data) {}

    unpacker(const std::byte* data, std::size_t size) : unpacker(size, data) {}
    unpacker(const char* data, std::size_t size) : unpacker(size, data) {}

    template <typename T, std::size_t N>
    unpacker(const T (&array)[N]) : unpacker(std::data(array), N) {}

    // Applying SFINAE to constructor templates is a bit unusual.  There is no
    // return value, and using SFINAE in the constructor arguments means the
    // template type deduction sometimes cannot deduce the argument type.  The
    // only place left to apply SFINAE is the template arguments, so we have a
    // dummy second template argument and apply the SFINAE to the default
    // argument type
    template <typename C, typename V = details::byte_container_t<C>>
    unpacker(const C& container)
        : unpacker(std::data(container), std::size(container)) {}

    [[nodiscard]] auto size() const { return m_end - m_begin; }
    [[nodiscard]] auto remaining() const { return m_end - m_next; }
    void reset() { m_next = m_begin; }

    unpacker& operator>>(unsigned char& c) {
        copy(&c, 1);
        return *this;
    }
    unpacker& operator>>(signed char& c) {
        copy(&c, 1);
        return *this;
    }
    unpacker& operator>>(char& c) {
        copy(&c, 1);
        return *this;
    }
    unpacker& operator>>(bool& b) {
        // need to force non-zero byte to true
        unsigned char c{};
        copy(&c, 1);
        b = !!c;
        return *this;
    }
    unpacker& operator>>(std::byte& b) {
        copy(&b, 1);
        return *this;
    }

    unpacker& operator>>(std::uint16_t& s) {
        copy(&s, 2);
        s = ntohs(s);
        return *this;
    }
    unpacker& operator>>(std::int16_t& s) {
        copy(&s, 2);
        s = ntohs(s);
        return *this;
    }

    unpacker& operator>>(std::uint32_t& l) {
        copy(&l, 4);
        l = ntohl(l);
        return *this;
    }

    unpacker& operator>>(std::int32_t& l) {
        copy(&l, 4);
        l = ntohl(l);
        return *this;
    }

    template <typename T, std::size_t N>
    details::unpackable_t<T> operator>>(T (&array)[N]) {
        T* tp{array};

        for (std::size_t n = N; n > 0; n--) {
            *this >> *tp++;
        }
        return *this;
    }

    template <typename T, std::size_t N>
    details::unpackable_t<T> operator>>(std::array<T, N>& array) {
        for (auto& a : array) {
            *this >> a;
        }
        return *this;
    }

   private:
    // Give these default values here to keep clang-tidy quiet, as the
    // clang-tidy rule cppcoreguidelines-pro-type-member-init does not
    // recognise template constructors delegating to other constructors. See
    // https://github.com/llvm/llvm-project/issues/37250

    iterator m_begin{};
    iterator m_end{};
    iterator m_next{};

    void check_size(std::size_t n) {
        if (m_next + n > m_end) throw std::length_error("unpacker overrun");
    }
    void copy(void* p, std::size_t n) {
        check_size(n);
        auto ucp = static_cast<unsigned char*>(p);
        while (n-- > 0) *ucp++ = *m_next++;
    }
};

}  // namespace james
#endif  // JAMES_UNPACKER_H
