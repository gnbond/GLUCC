#ifndef GLUCC_NULLABLE_FUNCTION_H
#define GLUCC_NULLABLE_FUNCTION_H

#include <functional>
#include <utility>

namespace glucc {

// The main template is undefined, only specializations for function types are
// defined
template <typename Func>
struct nullable_function;  // Error here means Func is not a function type

// Specialization for functions returning a type
template <typename Ret, typename... Args>
struct nullable_function<Ret(Args...)> : public std::function<Ret(Args...)> {
    Ret operator()(Args... args) {
        if (*this) {
            return std::function<Ret(Args...)>::operator()(
                std::forward<Args>(args)...);
        }
        return Ret{};
    }
};

// Further specialization for functions returning void
template <typename... Args>
struct nullable_function<void(Args...)> : public std::function<void(Args...)> {
    void operator()(Args... args) {
        if (*this) {
            return std::function<void(Args...)>::operator()(
                std::forward<Args>(args)...);
        }
    }
};

}  // namespace glucc
#endif  //  GLUCC_NULLABLE_FUNCTION_H
