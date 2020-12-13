cmake_minimum_required(VERSION 3.16)

project(bigmath LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG"${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

# Library

file(GLOB BIGMATH_SRC "bigmath/*.cpp")
add_library(bigmath STATIC ${BIGMATH_SRC})
target_compile_definitions(bigmath PRIVATE BIGMATH_BUILTIN=1)

# Tests

file(GLOB TESTS_SRC "tests/*.cpp")

add_executable(tests ${TESTS_SRC})
find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(tests Catch2::Catch2)
target_link_libraries(tests bigmath)
target_include_directories(tests PRIVATE ".")

# Performance Benchmarks

file(GLOB BENCHMARKS_SRC "benchmarks/*.cpp")
find_package(benchmark CONFIG REQUIRED)
add_executable(benchmarks-builtin ${BIGMATH_SRC} ${BENCHMARKS_SRC})
target_compile_definitions(benchmarks-builtin PRIVATE BIGMATH_BUILTIN=1
  BIGMATH_BENCHMARKS_NAME=builtin)
target_link_libraries(benchmarks-builtin benchmark::benchmark)
target_include_directories(benchmarks-builtin PRIVATE ".")

# BUILTIN    => __builtin_uaddl_overflow or uint128_t
# ASM_x86_64 => hand-written in asm
