#ifndef GLUCC_FORMAT_GUARD_H
#define GLUCC_FORMAT_GUARD_H
/**
@file format_guard.hpp
@author Gregory Bond (greg@bond.id.au)
@copyright This file is in the public domain.  See <https://unlicense.org>

This and other fine code available from https://github.com/gnbond/GLUCC

Comments / issues / pull requests gladly accepted
*/
#include <ios>

namespace glucc {

/**
@brief RAII guard for restoring IOStream formatting state

This is an RAII helper that saves the IOStream formatting state then
automatically reverts it on destruction.

IOStreams is type-safe and easily extensible, but for anything other than
default formatting of built-in types is both verbose and prone to unexpected
side-effects.  The most problematic side-effect is that most (but not all!) of
the I/O manipulators change the underling stream object, and hence will affect
all future operations unless manually reverted.

For example,
```
void print_hex( int i )
{
    std::cout << std::hex() << std::fill('0') << std::setw(4) << i;
    // Note that hex() and fill() still active, but setw() is not!
}
// Years later, in another file....
foo()
{
    int i = 26;
    // This will print "001a" as expected
    print_hex(i);
    // this will print "1a" which is most definitely _not_ expected!
    std::cout << ' is decimal ' << i;
}
```

The solution is to use `format_guard`:

```
void print_hex( int i )
{
    glucc::format_guard(std::cout);

    std::cout << std::hex() << std::fill('0') << std::setw(4) << i;
    // std::cout formatting is automatically restored to it's previous
configuration here
}
```

This template was somewhat influenced by the Boost state savers, but is intended
to be a much simpler solution to most (but not all) of the use-cases the Boost
classes addresses.  And it comes without the Boost overhead, of course.  See
https://www.boost.org/doc/libs/1_61_0/libs/io/doc/ios_state.html

This is a template to match the definition of basic_ios<>, so should also work
for wide strings or any custom stream type with custom character types (not that
I have ever heard of any such thing).  C++17 template type deductions mean we
don't need to explicitly define type aliases for each of the CharT types, as
iostreams does for std::basic_ios<> and the like.

@tparam CharT The character type for this stream
@tparam Traits The character traits for this stream
*/
template <typename CharT, typename Traits>
class format_guard {
   public:
    /**
     * @brief Capture the IO formatting state of the given stream
     *
     * @param ios The IOStream to save state for
     */
    format_guard(std::basic_ios<CharT, Traits>& ios)
        : m_ios{ios},
          m_fmtflags{ios.flags()},
          m_precision{ios.precision()},
          m_width{ios.width()},
          m_fill{ios.fill()} {}
    /**
     * @brief Destroy the format guard object
     *
     * This will restore the IOStream formatting parameters to the original
     * state
     */
    ~format_guard() {
        m_ios.flags(m_fmtflags);
        m_ios.precision(m_precision);
        m_ios.width(m_width);
        m_ios.fill(m_fill);
    }

    /**
     * @brief Cannot be copied or assigned
     * @name Copy/Assign
     */
    ///@{
    format_guard(const format_guard&) = delete;
    format_guard(format_guard&&) = delete;
    format_guard& operator=(const format_guard&) = delete;
    format_guard& operator=(format_guard&&) = delete;
    ///@}

   private:
    std::basic_ios<CharT, Traits>& m_ios;
    std::ios_base::fmtflags m_fmtflags;
    std::streamsize m_precision;
    std::streamsize m_width;
    CharT m_fill;
};

}  // namespace glucc

#endif  // GLUCC_FORMAT_GUARD_H
