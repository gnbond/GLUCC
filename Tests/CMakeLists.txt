
# Dynamically dowload the Catch2 framework
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.3.1 
)

FetchContent_MakeAvailable(Catch2)

# Having downloaded Catch2, import the CMake and CTest integration 
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

set(TEST_FILES
    test_nullable_function.cpp
    test_format_guard.cpp
)

add_executable(UnitTests ${TEST_FILES} )
target_link_libraries(UnitTests PRIVATE Catch2::Catch2WithMain)
# TO find the source headers
target_include_directories(UnitTests PRIVATE ..)

catch_discover_tests(UnitTests)
