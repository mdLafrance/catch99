cmake_minimum_required(VERSION 3.10)

project(
  Catch99
  VERSION 1.0.0
  DESCRIPTION "Catch99 self-testing suite"
  LANGUAGES C)

find_program(MOLD_EXECUTABLE mold)

# Use mold if available
if(MOLD_EXECUTABLE)
  message(STATUS "found mold linker: ${MOLD_EXECUTABLE}")
  if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Compiling for MSVC")
    set(ARGS_ALL
        "${ARGS_ALL} -fuse-ld=mold -DCMAKE_LINKER=${MOLD_EXECUTABLE} -DCMAKE_C_LINK_EXECUTABLE=${MOLD_EXECUTABLE}"
    )
  elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(STATUS "Compiling with clang")
    set(ARGS_ALL
        "${ARGS_ALL} -B mold -DCMAKE_LINKER=${MOLD_EXECUTABLE} -DCMAKE_C_LINK_EXECUTABLE=${MOLD_EXECUTABLE}"
    )
  else()
    message(STATUS "Compiling with gcc (or other)")
    set(ARGS_ALL "${ARGS_ALL} -fuse-ld=mold")
  endif()
else()
  message(STATUS "mold not found")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

aux_source_directory(tests TEST_FILES)

add_executable(test ${TEST_FILES})
target_include_directories(test PRIVATE include)
