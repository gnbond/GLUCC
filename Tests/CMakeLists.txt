
set(TEST_FILES
    test_nullable_function.cpp
)

add_executable(Test main.cpp ${TEST_FILES} )
# Find the included version of catch2
target_include_directories(Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

add_test(NAME UnitTests COMMAND Test)
