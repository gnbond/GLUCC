#include "nullable_function.hpp"

#include <catch2/catch.hpp>

using Int_Func = glucc::nullable_function<int()>;
using Void_Func = glucc::nullable_function<void()>;
using Int_Arg_Func = glucc::nullable_function<int(int)>;
using Double_Args_Func = glucc::nullable_function<double(double, int)>;

static int ret_two() { return 2; }

bool ret_void_called = false;
static void ret_void() { ret_void_called = true; }

static int ret_twice(int i) { return 2 * i; }
static double ret_times(double val, int mul) { return val * mul; }

struct Four {
    Four(int val) : m_val{val} {}
    int get() { return m_val; }
    int get_multiple(int factor) { return m_val * factor; }
    double get_multiple_plus(double factor, int add) {
        return m_val * factor + add;
    }

   private:
    int m_val;
};

TEST_CASE("nullable_function Basic", "[nullable_function]") {
    Int_Func f{};

    CHECK_FALSE(f);
    f();              // This compiles and is safe
    CHECK(f() == 0);  // And returns the default-constructed Ret type

    // From a pointer-to-function
    Int_Func f2{ret_two};
    CHECK(f2);
    CHECK(f2() == 2);

    // From a lambda
    Int_Func f3{[]() { return 3; }};
    CHECK(f3);
    CHECK(f3() == 3);

    // From a bind
    Four four{4};
    CHECK(four.get() == 4);

    Int_Func f4{std::bind(&Four::get, &four)};
    CHECK(f4);
    CHECK(f4() == 4);
}

TEST_CASE("nullable_function Void", "[nullable_function]") {
    Void_Func f{};
    CHECK_FALSE(f);
    f();  // This compiles and is safe

    // From a pointer-to-function
    Void_Func f2{ret_void};
    CHECK(f2);
    ret_void_called = false;
    f2();  // Compiles and runs
    CHECK(ret_void_called);

    // From a lambda
    int val = 3;
    Void_Func f3{[&val]() { val = 33; }};
    CHECK(f3);
    f3();
    CHECK(val == 33);
}

TEST_CASE("nullable_function With Argument", "[nullable_function]") {
    Int_Arg_Func f{};

    CHECK_FALSE(f);
    f(3);              // This compiles and is safe
    CHECK(f(3) == 0);  // And returns default value for Ret

    // From a pointer-to-function
    Int_Arg_Func f2{ret_twice};
    CHECK(f2);
    CHECK(f2(5) == 10);

    // From a lambda
    Int_Arg_Func f3{[](int i) { return 3 * i; }};
    CHECK(f3);
    CHECK(f3(2) == 6);

    // From a bind
    using namespace std::placeholders;

    Four four{4};
    CHECK(four.get() == 4);

    Int_Arg_Func f4{std::bind(&Four::get_multiple, &four, _1)};
    CHECK(f4);
    CHECK(f4(3) == 12);
}

TEST_CASE("nullable_function With Two Arguments", "[nullable_function]") {
    Double_Args_Func f{};

    CHECK_FALSE(f);
    CHECK(f(4.0, 3) ==
          0.0);  // This compiles and is safe and returns default val

    // From a pointer-to-function
    Double_Args_Func f2{ret_times};
    CHECK(f2);
    CHECK(f2(3.5, 5) == 17.5);

    // From a lambda
    Double_Args_Func f3{[](double d, int i) { return d * i; }};
    CHECK(f3);
    CHECK(f3(3.5, 3) == 10.5);

    // From a bind
    using namespace std::placeholders;

    Four four{4};
    CHECK(four.get() == 4);

    Double_Args_Func f4{std::bind(&Four::get_multiple_plus, &four, _1, _2)};
    CHECK(f4);
    CHECK(f4(0.5, 3) == 5.0);  // 4 * 0.5 + 3
}
