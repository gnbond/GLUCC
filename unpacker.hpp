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
}  // namespace details

class unpacker {
   public:
    // unsigned char is the default packet data type
    unpacker(const unsigned char* data, std::size_t size)
        : m_data{data}, m_size{size} {}

    unpacker(const std::byte* data, std::size_t size)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        : unpacker{reinterpret_cast<const unsigned char*>(data), size} {}
    unpacker(const char* data, std::size_t size)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        : unpacker{reinterpret_cast<const unsigned char*>(data), size} {}

    template <typename T, std::size_t N>
    // clang-tidy bug, See https://github.com/llvm/llvm-project/issues/37250
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    unpacker(const T (&array)[N]) : unpacker(std::data(array), N) {}

    // Applying SFINAE to constructor templates is a bit unusual.  There is no
    // return value, and using SFINAE in the constructor arguments means the
    // template type deduction sometimes cannot deduce the argument type.  The
    // only place left to apply SFINAE is the template arguments, so we have a
    // dummy second template argument and apply the SFINAE to the default
    // argument type
    template <typename C, typename V = details::byte_container_t<C>>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    unpacker(const C& container)
        : unpacker(std::data(container), std::size(container)) {}

    [[nodiscard]] auto size() const { return m_size; }
    [[nodiscard]] auto remaining() const { return m_size - m_next; }
    void reset() { m_next = 0; }

    unpacker& operator>>(unsigned char& c) {
        copy(&c, 1);
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

   private:
    const unsigned char* m_data;
    std::size_t m_size;
    std::size_t m_next{0};

    void check_size(unsigned n) {
        if (m_next + n > m_size) throw std::length_error("unpacker overrun");
    }
    void copy(void* p, std::size_t n) {
        check_size(n);
        auto ucp = static_cast<unsigned char*>(p);
        auto dp = m_data + m_next;
        m_next += n;
        while (n-- > 0) *ucp++ = *dp++;
    }
};

}  // namespace james
#endif  // JAMES_UNPACKER_H
