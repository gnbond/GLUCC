# Test that thing which sould fail to compile, do actually fail to compile, and for the reaons we expect.

# Ideas from https://ibob.bg/blog/2022/10/04/testing-build-failure-with-cmake/
# But ive implemented this using shell script for the actual test, not cmake,
# and this implementation is ver specific to the GLUCC library (and running the tests under linux).

function(add_nocompile_test filename )
    get_filename_component(base ${filename} NAME_WE )

    add_executable( ${base}-build EXCLUDE_FROM_ALL ${filename} )
    # TO find the source headers
    target_include_directories(${base}-build PRIVATE ../..)

    add_test(NAME ${base}-nocompile 
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/nocompile.sh ${CMAKE_CURRENT_SOURCE_DIR}/${filename})
endfunction()

# CTest swallows the stderr / stdout from the child shell script which can make debugging a bit tricky.
# IF a no-compile test for foo.cpp fails, then you can work out why by manually building the target foo-build

# test code files should include 1 or more lines like 
#
#  // expect: compiler error message
# 
# The no-compile test will be considered success if the build fails and the
# string "compiler error message" appears in stderr An=y other circumstance
# (build succeeding, no expect lines, build failing with a different error) will
# be considered failure of the no-compile test 

# For the below tests, the expected failure messages are from clang11

add_nocompile_test( packer_no_inserter.cpp )
add_nocompile_test( packer_no_push_back.cpp )
add_nocompile_test( packer_long_insert.cpp )

add_nocompile_test( optional_function_not_function.cpp )
add_nocompile_test( optional_function_not_function2.cpp )
