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
  const uint64_t reserved = 16;

  auto nat = bigmath::nat_new<HeapAllocator>(0ul, size + reserved);
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
  const uint32_t words_count = 8;

  uint64_t words[words_count];
  for (uint64_t i = 0; i < words_count; ++i) {
    words[i] = (i % 4 == 0) ? ~0ul : i;
  }

  uint64_t offset = 0;
  auto a = make_nat(state.range(0));

  for (auto _ : state) {
    for (uint64_t i = 0; i < words_count; ++i)
      a = bigmath::nat_add_word(a, words[(offset + i) & (words_count - 1)]);
    offset++;
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
