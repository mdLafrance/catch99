cmake_minimum_required(VERSION 3.10)

project(Catch99 VERSION 1.0.0 DESCRIPTION "Catch2 but for C99" LANGUAGES C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

aux_source_directory(tests TEST_FILES)

# Tests
add_executable(test ${TEST_FILES})
target_include_directories(test PRIVATE include)
