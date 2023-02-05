

add_executable(Test main.cpp)
# Find the included version of catch2
target_include_directories(Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

