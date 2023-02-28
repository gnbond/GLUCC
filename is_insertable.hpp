#ifndef GLUCC_IS_INSERTABLE_H
#define GLUCC_IS_INSERTABLE_H

/**
 * @file is_insertable.hpp
 * @author Gregory Bond (greg@bond.id.au)
 * @copyright This file is in the public domain.  See <https://unlicense.org>
 *
 * This and other fine code available from https://github.com/gnbond/GLUCC
 *
 * Comments / issues / pull requests gladly accepted
 */

#include <type_traits>

/*
 * Older LLVM libcpp ostreams versions (in particular, as shipped with the clang
 * version in MacOs XCode 10) have a bug that means the usual C++17 SFINAE idiom
 * does not work correctly for std::ostream.
 *
 * On these systems, `decltype(std::declval<std::ostream>() <<
 * std::declval<T>())` does not work as expected if `T` is not an insertable
 * type. This (correctly) fails outside of declval(), but the template
 * `operator<<(Stream&&, const T&)` this is ultimately referencing is not
 * sufficiently constrained so inside decltype() the SFINAE does not correctly
 * propagate up to the template specialization.  End result is that the
 * resulting trait gives false positives for std::ostream.
 *
 * So far I have found this bug in libcpp shipped with XCode 10 (v 6000) and
 * also in a homebrew-installed llvm11 (v11000), but it is not present in XCode
 * 14. We detect that here and use an alternate idiom for those compilers.
 *
 * As far as I can see, gcc's libstdc++ does not suffer from this problem.
 *
 * See
 * https://github.com/llvm/llvm-project/commit/fdc41e11f9687a50c97e2a59663bf2d541ff5489
 */

#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION <= 11000
#define BROKEN_LIBCPP
#endif

namespace glucc {

/**

@brief Base template

This is the base template, and it establishes two things:
 - Any 2-argument instantiation is_insertable_into<T,S> is actually
   instantiating is_insertable_into<T,S,void>.  The application of default
   template arguments happens very early in the template instantiation process,
   before any possible specializations are considered

 - The default answer for is_insertable_into is false_type
 */
template <typename T, typename S, typename U = void>
struct is_insertable_into : std::false_type {};

#ifndef BROKEN_LIBCPP

/**
@brief This specialization for the true case

If all the type arguments to `std::void_t<>` are well-formed, then this declares
a partial specialization is_insertable_into<T,S,void>.

The users of this trait will specify is_insertable_into<T, S>, which (due to the
default parameter) is actually is_insertable_into<T,S,void>.

Compiler will prefer this specialization if it is well-formed, as <T,S,void> is
a closer match that <T,S,U>, and the result is `true_type`.  If any of the type
arguments to std::void_t are not well-formed, then the SFINAE rules remove this
specialization and the `false_type` main template is used.

We check two conditions in the void_t<>:
 - That, for stream `s` of type `S` and value `t` of type `T`, the expression `s
   << t` is well-formed.
 - That S is not an integral type, as `1 << 3` is also well-formed but does not
   represent insertion into a stream

@tparam T The type to insert
@tparam S The stream type
*/
template <typename T, typename S>
struct is_insertable_into<
    T, S,
    std::void_t<decltype(std::declval<S>() << std::declval<T>()),
                std::enable_if_t<!std::is_integral_v<S>>>> : std::true_type {};

#else

/*
As a non-idiomatic work-around, we declare (but do not define) a function to
return `S&` and use that in `decltype()`.  This is more-or-less equivalent to
`std::declval<S>()` but does not trigger the bug in XCode-10 libstdc++.
*/
template <typename S>
S& S_func();

template <typename T, typename S>
struct is_insertable_into<
    T, S,
    std::void_t<decltype(S_func<S>() << std::declval<T>()),
                std::enable_if_t<!std::is_integral_v<S>>>> : std::true_type {};

#endif

/**
 * @brief Helper variable template  Is a value of type T insertable into a
 * stream of type S?
 *
 * @tparam T The type of the expression to insert
 * @tparam S The type of the stream
 */
template <typename T, typename S>
constexpr bool is_insertable_into_v = is_insertable_into<T, S>::value;

}  // namespace glucc
#endif  //  GLUCC_IS_INSERTABLE_H
