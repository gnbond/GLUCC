cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)

project(GLUCC LANGUAGES CXX VERSION 1.0)

set(HEADERS 
    optional_function.hpp
    format_guard.hpp
    packer.hpp
    unpacker.hpp
    is_insertable.hpp
)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    # Doxygen markdoun formatter is not totally compatible with GitHub. So we
    # build a derived README.md and process that 
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/README.md
        COMMAND sed 
            -e s/^<.--.toc.*/[TOC]/ 
            -e /^<.--.begin/,/^$/d 
            -e s/<.--.*>// 
            -e s/```C++/```.cpp/
             ${CMAKE_CURRENT_SOURCE_DIR}/README.md > ${CMAKE_CURRENT_BINARY_DIR}/README.md
        MAIN_DEPENDENCY README.md
        VERBATIM
    )
    set(DOXYGEN_HTML_OUTPUT .)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_BINARY_DIR}/README.md)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen)
    # Don't document the details:: namespace
    set(DOXYGEN_EXCLUDE_SYMBOLS "*::details::*")
    doxygen_add_docs(Doxygen
        ${HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/README.md
        LICENSE.md
    )
else()
    message(INFO "Doxygen not found")
endif()

enable_testing()
add_subdirectory(Tests)
