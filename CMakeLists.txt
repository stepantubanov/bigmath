cmake_minimum_required(VERSION 3.19)

project(bigmath LANGUAGES CXX ASM VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -g")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Detect capabilities.

set(BIGMATH_CONFIGURATIONS "")

list(APPEND BIGMATH_CONFIGURATIONS asm_x86_64)
if(NOT MSVC)
  list(APPEND BIGMATH_CONFIGURATIONS builtin_int128)
  #list(APPEND BIGMATH_CONFIGURATIONS builtin_overflow)
endif()

list(GET BIGMATH_CONFIGURATIONS 0 BIGMATH_CONFIGURATION)

# Library

string(TOUPPER ${BIGMATH_CONFIGURATION} BIGMATH_CONFIGURATION_UPPER)
set(BIGMATH_DEFINITIONS "BIGMATH_${BIGMATH_CONFIGURATION_UPPER}=1")
file(GLOB BIGMATH_SRC "bigmath/*.cpp" "bigmath/*.s")
add_library(bigmath STATIC ${BIGMATH_SRC})
target_compile_definitions(bigmath PRIVATE ${BIGMATH_DEFINITIONS})

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

foreach(configuration IN LISTS BIGMATH_CONFIGURATIONS)
  string(TOUPPER ${configuration} configuration_uppercase)

  add_executable(benchmark_${configuration} ${BIGMATH_SRC} ${BENCHMARKS_SRC})
  target_compile_definitions(benchmark_${configuration} PRIVATE
    BIGMATH_${configuration_uppercase}=1
    BIGMATH_CONFIGURATION=${configuration})
  target_link_libraries(benchmark_${configuration} benchmark::benchmark)
  target_include_directories(benchmark_${configuration} PRIVATE ".")
endforeach()
