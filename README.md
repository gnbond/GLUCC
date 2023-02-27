<!---
The "omit from toc" comment for the Markdown VSCode extension unfortunately 
gets rendered in the Doxygen-generated HTML.  We work around this in CMake to generate 
README.md with the comments removed and include that in Doxygen run
Also the TOC maintained by VSCode is not compatible with the Markdown way
so the same CMake magic adjusts that
Also syntax highlighting of code blocks is different
Base strategy is this file is suitable for VSCode and Github, we massage it 
as required for Doxygen
-->
# GLUCC - Greg's List of Useful C++ Classes <!-- omit from toc -->

<!-- toc this line is replaced by Doxygen [TOC] tag -->
<!-- begin toc this and the following up to blank line is deleted -->
- [1. Introduction](#1-introduction)
- [2. Documentation](#2-documentation)
- [3. Support](#3-support)
- [4. The Templates](#4-the-templates)
  - [4.1. format\_guard](#41-format_guard)
  - [4.2. optional\_function](#42-optional_function)
  - [4.3. is\_insertable\_into](#43-is_insertable_into)
  - [4.4. dereference\_iterator](#44-dereference_iterator)
  - [4.5. rectangular](#45-rectangular)
- [5. The Classes](#5-the-classes)
  - [5.1. kerry::packer](#51-kerrypacker)
  - [5.2. james::unpacker](#52-jamesunpacker)


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

The documentation is in the `docs/` directory, and can be accessed online via
[GLUCC Github Page](https://gnbond.github.io/GLUCC/).   Doxygen-generated API is
in the [docs/doxygen](docs/doxygen/index.html) directory.

# 3. Support

The long-term home of this project is on [GitHub](https://github.com/gnbond/GLUCC).

As stated in the [License](LICENSE.md), this code is provided as-is with absolutely
no warranty.  That said, I am always happy to receive comments, suggestions,
pull requests or issues on GitHub, tho I cannot promise timely responses.

# 4. The Templates

## 4.1. format_guard

An [RAII](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii)
helper that saves the IOStream formatting state then automatically reverts it on
destruction.

## 4.2. optional_function

A `std::function` wrapper that is safe to call even if empty.

## 4.3. is_insertable_into

A type trait to detect if a type `T` is insertable into an iostreams-like stream
`S`.  This demonstrates some useful Template Meta-Programming techniques that can
be simply adapted to implement a wide variety of type traits.

## 4.4. dereference_iterator 

TBD

## 4.5. rectangular

TBD

# 5. The Classes

## 5.1. kerry::packer

Construct binary protocol packets in C++ style, without needing `memcpy()` or unsafe pointer casts.  Still a work in progress.

## 5.2. james::unpacker

Unpack binary protocol packet in C++ style.  TBD.
