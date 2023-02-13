#ifndef GLUCC_OPTIONAL_FUNCTION_H
#define GLUCC_OPTIONAL_FUNCTION_H

/**
 * @file optional_function.hpp
 * @author Gregory Bond (greg@bond.id.au)
 * @copyright This file is in the public domain.  See <https://unlicense.org>
 *
 * This and other fine code available from https://github.com/gnbond/GLUCC
 *
 * Comments / issues / pull requests gladly accepted
 */

#include <functional>
#include <utility>

namespace glucc {

/**
 * @brief Wrapper for `std::function` that can be safely left empty
 *
 * The
 * [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function)
 * template is extremely useful but has one annoying feature when used in some
 * designs.  A `std::function` can be _empty_, and default-constructed objects
 * are created as _empty_.  An _empty_ std::function is more or less equivalent
 * to a null pointer, and will throw an exception if dereferenced.
 *
 * So for designs where the function is optional (such as when used for a
 * callback), the code is full of constructs like
 * ```
 *  std::function<void()> m_func{};
 *  // ...
 *  if (m_func)
 *  {
 *       m_func();
 *  }
 * ```
 * and you risk exceptions if you forget to add this check on some code paths.
 * Such errors can be very hard to find and are likely to escape testing into
 * deployed code.
 *
 * `optional_function` automates this test, so removing it from the application
 * code and ensuring that _empty_ function objects never throw exceptions
 * (though the wrapped function may still throw, of course):
 * ```
 * glucc::optional_function<void()> m_func{};
 * // ...
 * m_func();  // Always safe
 * ```
 *
 * If the function type returns a non-void type, then an empty
 * `optional_function` will return a default-constructed value of that type.
 *
 * The base template is incomplete and should not be used.  Template Partial
 * Specialisation will choose the correct code based on the function type.  If
 * you get errors such  as this:
 * ```
 * file.cpp:132:32: error: implicit instantiation of undefined template
 * 'glucc::optional_function<int>'
 * ```
 * this likely means you are instantiating `optional_function` with a type that
 * is not a function type.
 *
 * @tparam Func Function type to wrap, e.g. `double(int, double)`
 */
template <typename Func>
struct optional_function;  // Error here means Func is not a function type

/**
 * @brief Wrap a non-void function
 *
 * @tparam Ret Return type - must be default-constructable
 * @tparam Args... Varadic function argument types
 */
template <typename Ret, typename... Args>
struct optional_function<Ret(Args...)> : public std::function<Ret(Args...)> {
    /**
     * @brief Invoke the wrapped function
     *
     * If the optional_function is _empty_, returns `Ret{}`.  Otherwise returns
     * the result from from invoking the function.
     *
     * @param args Varadic function arguments
     * @return Ret Returned value from wrapped function
     */
    Ret operator()(Args... args) {
        if (*this) {
            return std::function<Ret(Args...)>::operator()(
                std::forward<Args>(args)...);
        }
        return Ret{};
    }
};

/**
 * @brief Wrap a function returning void
 *
 * @tparam Args... Varadic function argument types
 */
template <typename... Args>
struct optional_function<void(Args...)> : public std::function<void(Args...)> {
    /**
     * @brief Invoke the wrapped function
     *
     * If the optional_function is _empty_, then this is a no-op.
     *
     * @param args Varadic function arguments
     */
    void operator()(Args... args) {
        if (*this) {
            return std::function<void(Args...)>::operator()(
                std::forward<Args>(args)...);
        }
    }
};

}  // namespace glucc
#endif  //  GLUCC_OPTIONAL_FUNCTION_H
