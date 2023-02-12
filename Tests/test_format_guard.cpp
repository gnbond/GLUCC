#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "format_guard.hpp"

using namespace glucc;

TEST_CASE("format_guard basic", "[format_guard]") {
    format_guard cout_guard{std::cout};
    format_guard cin_guard{std::cin};
}

TEST_CASE("format_guard wchar", "[format_guard]") {
    std::wostringstream woss;
    std::wistringstream wiss;

    format_guard wo_guard{woss};
    format_guard wi_guard{wiss};
}

// We choose hex() as a representitive of the ios_base::fomtflags
TEST_CASE("format_guard flags", "[format_guard]") {
    std::ostringstream oss;
    {
        format_guard guard{oss};
        oss << std::hex << std::showbase << 26;
    }
    oss << ' ' << 26;
    CHECK(oss.str() == "0x1a 26");
}

TEST_CASE("format_guard fill", "[format_guard]") {
    std::ostringstream oss;
    {
        format_guard guard{oss};
        oss << std::setfill('x') << std::setw(4) << 6;
    }
    oss << std::setw(4) << 26;
    CHECK(oss.str() == "xxx6  26");
}

TEST_CASE("format_guard precision", "[format_guard]") {
    std::ostringstream oss;
    {
        format_guard guard{oss};
        oss << std::setprecision(4) << M_PI;
    }
    oss << ' ' << M_PI;
    // Remember, precision also includes the digits before the decimal point
    CHECK(oss.str() == "3.142 3.14159");
}

TEST_CASE("format_guard wstr flags/fill/function", "[format_guard]") {
    std::wostringstream woss;

    {
        format_guard guard{woss};

        woss << std::hex << std::setfill(L'0') << std::setw(4) << 26;
    }
    woss << " " << 26;
    CHECK(woss.str() == L"001a 26");
}

// Check that some notes in the documentation actually do what the documentation
// claims!
TEST_CASE("format_guard documentation", "[format_guard]") {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(4) << 26;
    oss << " " << 26;
    CHECK(oss.str() == "001a 1a");
}
