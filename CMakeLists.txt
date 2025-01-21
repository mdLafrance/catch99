cmake_minimum_required(VERSION 3.10)

project(
  Catch99
  VERSION 1.0.0
  DESCRIPTION "Catch99 self-testing suite"
  LANGUAGES C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

aux_source_directory(tests TEST_FILES)

add_executable(test ${TEST_FILES})
target_include_directories(test PRIVATE include)
# Link against your project's source
# target_link_libraries(test PRIVATE libfoo)
