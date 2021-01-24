### Work In Progress.

Code for working with arbitrary precision numbers. CMake project includes the
tests executable, bigmath static library and two benchmarks (one contains x86-64
hand-written assembly routines, another uses compiler builtins).

Benchmarks require Google Benchmark library installed.

```
cmake -DCMAKE_BUILD_TYPE=Release -B build .
cd build && make

./tests                     # run tests

./benchmark_asm_x86_64      # run x86-64 version (google benchmark)
./benchmark_builtin_int128  # run builtin version (google benchmark)
```
