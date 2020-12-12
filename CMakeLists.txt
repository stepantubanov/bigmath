cmake_minimum_required(VERSION 3.16)

project(bigmath LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG"${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

# Library

# TODO: this will be using chosen cmake configuration variables.

file(GLOB BIGMATH_SRC "bigmath/*.h" "bigmath/*.cpp")
add_library(bigmath STATIC ${BIGMATH_SRC})

# Tests

file(GLOB TESTS_SRC "tests/*.h" "tests/*.cpp")

add_executable(tests ${TESTS_SRC})
find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(tests Catch2::Catch2)
target_link_libraries(tests bigmath)
target_include_directories(tests PRIVATE "bigmath")

# Performance Benchmarks

# TODO: this will be building and running performance benchmarks for
# multiple executables.

# BUILTIN    => __builtin_uaddl_overflow or uint128_t
# ASM_x86_64 => hand-written in asm
