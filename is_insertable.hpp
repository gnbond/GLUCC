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
 - That, for lvalue `s` of type `S` and value `t` of type `T`, the expression `s
   << t` is well-formed.
 - That S is not an integral type, as `1 << 3` is also well-formed but does not
   represent insertion into a stream

@tparam T The type to insert
@tparam S The stream type
*/
template <typename T, typename S>
struct is_insertable_into<
    T, S,
    std::void_t<decltype(std::declval<S&>() << std::declval<T>()),
                std::enable_if_t<!std::is_integral_v<S>>>> : std::true_type {};

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
