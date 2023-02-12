
# GLUCC - Greg's List of Useful C++ Classes <!-- omit from toc -->

- [1. Introduction](#1-introduction)
- [2. Documentation](#2-documentation)
- [3. Support](#3-support)
- [4. Why GLUCC?](#4-why-glucc)
- [5. Code style](#5-code-style)
- [6. The Templates](#6-the-templates)
  - [6.1. format\_guard](#61-format_guard)
  - [6.2. nullable\_function](#62-nullable_function)
  - [6.3. dereference\_iterator](#63-dereference_iterator)
  - [6.4. rectangular](#64-rectangular)
- [7. Some other Useful C++ Classes](#7-some-other-useful-c-classes)
  - [7.1. KissNet](#71-kissnet)
  - [7.2. CRC++](#72-crc)


# 1. Introduction

This is a small collection of C++17 classes and templates that I have found
useful in some of my personal projects.   I've gathered them here in one place,
with a very forgiving license, in the hope they may be useful to others.

The intent is that each class or template is more or less stand-alone with few
or no dependencies.  With the forgiving license, files (and their unit tests!)
can just be copied into a target project and used.  No need to install an entire
library or manage an extra dependency just to get some small helper classes.

Each class is documented in the header with Doxygen.  Each class should be
comprehensively unit-tested using [Catch2](https://github.com/catchorg/Catch2).
A CMake-based build is included to built the documentation and unit tests but is
not required to use any of these classes in your project.

# 2. Documentation

  Doxygen-generated API is in the [here](doxygen/index.html).

# 3. Support

The long-term home of this project is on [GitHub](https://github.com/gnbond/GLUCC).

As stated in the [License](LICENSE.md), this code is provided as-is with absolutely
no warranty.  That said, I am always happy to receive comments, suggestions,
pull requests or issues on GitHub, tho I cannot promise timely responses.

# 4. Why GLUCC?

There are plenty of other class libraries out there, why a new one?

Firstly, and most importantly, because creating well-designed and useful library
classes is _fun_, and it is a skill that can always be improved.  Making the
results available to all allows others to give a different perspective that
hopefully can improve designs or implementation, and in turn hopefully I can
learn new stuff as well.

Secondly, this allows me to experiment with tools and language features that I
might not get to use in my day job (which is currently also C++ coding.)  For
example I don't do a lot of template hackery on my current project so some of
these templates have been a useful exercise exploring Template Meta-Programming.
Restricting support to C++17 (and later) means I can use new language features
without worrying about how ten year old compilers might handle them.

Thirdly, most of the existing class collections are either very restrictively
licensed (GNU, I'm looking at you) or heavyweight installations that require a
lot of dependency management (yes, Boost, I'm talking about you), or for other
reasons not easy to extract one or two small pieces for use in your project. If
you're already drinking the Boost Kool-Aid, there's probably nothing of interest
here.  If you've ever thought "I really want an IOStream format guard but I
don't want to install 500Mb of Boost to get it", maybe GLUCC can help.

Some of the classes here are inspired by classes found other libraries, or by
code snippets on StackOverflow or such, I have attributed those sources in code
comments, where appropriate.

The name was chosen to be pronounceable and hopefully not clash with other
similar collections of classes, but still be hopefully at least a little bit
evocative of what this library contains.  And who knows, if this really takes
off we can rename it "General Library of Useful C++ Classes".

# 5. Code style

Code is written to C++17 standard, and while most should work on C++11, this is
not guaranteed.  I've chosen not to use any C++20 features, even though ranges
and concepts would make some of these templates cleaner, as even now (Jan 2023)
C++20 support is not widespread and using C++20 features might restrict the
usefulness in projects that are not yet compiled with C++20 compilers.  And if
you are still using C++03, heck that's now 20 years old, so sorry, not
supported.

All the definitions are in the `glucc::` namespace.  Identifiers are
`lower_case_underscore`, to match the naming style of the standard library. Code
is (will be!) checked with `clang-tidy` and formatted with `clang-format`, and
the settings for these tools are in the root of the project.  

Coding style and conventions have been mostly inspired by Sutter & Stroustrup's
[CPP Core
Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines), which
should be required reading for any modern C++ programmer.

However you are obviously free (and encouraged) to change the namespace, the
naming and coding style to match the rest of your project when you copy a file
into your project.  Most components are quite small so this should not be a big
burden.

The unit tests use CMake FetchContent to download the Catch2 framework from
[Catch2 GutHub page](https://github.com/catchorg/Catch2) at configure time. This
means Configure takes a while, and the first build also takes a while as the
Catch2 code is compiled, but after that building and running unit tests is very
fast (much faster than the single-header version of Catch2 2.x).

# 6. The Templates

## 6.1. format_guard

An [RAII](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii)
helper that saves the IOStream formatting state then automatically reverts it on
destruction.

## 6.2. nullable_function

A `std::function` wrapper that is safe to call even if empty

## 6.3. dereference_iterator

## 6.4. rectangular

# 7. Some other Useful C++ Classes

The following are not part of GLUCC, but have similar philosophy - small,
targeted, mostly-standalone modules on modern C++ that can be easily copied into
an existing project.

These projects are quite independent of (and probably completely unaware of)
GLUCC.  Listing here should not imply any form of endorsement of GLUCC by those
projects.

## 7.1. KissNet

[KissNet](https://github.com/Ybalrid/kissnet) is a single-header standalone wrapper to make the C socket API easily and naturally available to C++ code.  Supports UDP, TCP and SSL, both clients and servers.  Also supports (spit) Windows.  MIT license.

## 7.2. CRC++

[CRC++](https://github.com/d-bahr/CRCpp) is a single-header solution for fast
calculation of all manner of CRCs (40+ CRC polynomials included, from 6 to 64
bits).  BSD License.
