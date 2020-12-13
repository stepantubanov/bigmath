#include <benchmark/benchmark.h>
#include <bigmath/natural.h>
#include <string.h>

struct HeapAllocator {
  static inline void* alloc(uint64_t size) { return ::malloc(size); }
  static inline void* realloc(void* ptr, uint64_t size) {
    return ::realloc(ptr, size);
  }
  static inline void free(void* ptr) { ::free(ptr); }
};

bigmath::natural<HeapAllocator>* make_nat(uint64_t size) {
  auto nat = bigmath::nat_new<HeapAllocator>(0ul);
  nat = bigmath::nat_reserve<HeapAllocator>(nat, size);
  nat->places_count = size;

  for (uint32_t i = 0; i < size; ++i) {
    if (i % 7 == 0)
      nat->places[i] = ~0ul;
    else
      nat->places[i] = i;
  }

  return nat;
}

static void nat_add_word(benchmark::State& state) {
  auto a = make_nat(state.range(0));

  for (auto _ : state) {
    a = bigmath::nat_add_word(a, 3ul);
    a = bigmath::nat_add_word(a, ~0ul);
    benchmark::DoNotOptimize(a->places[a->places_count - 1]);
  }

  state.counters["value"] = a->places[a->places_count - 1];
  bigmath::nat_free(a);
}

static void nat_add_nat(benchmark::State& state) {
  auto a = make_nat(state.range(0));
  auto b = make_nat(state.range(0));

  for (auto _ : state) {
    a = bigmath::nat_add_nat(a, b);
    benchmark::DoNotOptimize(a->places[a->places_count - 1]);
  }

  state.counters["value"] = a->places[a->places_count - 1];

  bigmath::nat_free(a);
  bigmath::nat_free(b);
}

BENCHMARK(nat_add_word)->Range(2, 100);
BENCHMARK(nat_add_nat)->Range(2, 4000);
BENCHMARK_MAIN();
