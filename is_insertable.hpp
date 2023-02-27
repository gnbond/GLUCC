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
 * Older MacOS versions (in particular, the clang version in MacOs XCode 10
 * which is the latest supported on OsX 10.13) have a bug that means the usual
 * C++17 SFINAE idiom using declval<>(), decltype() and std::void_t does not
 * work correctly.  We detect that here and use an alternate idiom for those
 * compilers.  The ugly #define is injected by the XCode version, we assume
 * XCode 11 for OsX 10.14 and beyond are OK, but I can't test that.
 *
 * On these systems, `decltype(std::declval<S>() << std::declval<T>())` does not
 * work as expected (SFINAE does not correctly propagate up to the template
 * specialization so the resulting trait gives false positives). It works if the
 * `std::declval<S>()` is replaced by either an actual variable (`std::cout` or
 * even `S{}`), but this limits the trait to S types that are
 * default-constructable.
 *
 * The problem appears to be related to XCode version or possibly OS version, as
 * installing a clang-11 from Homebrew fails to fix the problem.
 */
#if defined(__APPLE__) && defined(__clang__) && \
    __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ <= 101300
#define BROKEN_DECLTYPE
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

#ifndef BROKEN_DECLTYPE

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
`std::declval<S>()` but does not trigger the bug in XCode-10 clang.
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
