#include <benchmark/benchmark.h>
#include <bigmath/natural.h>
#include <string.h>

struct HeapAllocator {
  static inline void* alloc(u64 size) { return ::malloc(size); }
  static inline void* realloc(void* ptr, u64 size) {
    throw "Should not realloc during benchmark";
    return ::realloc(ptr, size);
  }
  static inline void free(void* ptr) { ::free(ptr); }
};

bigmath::natural<HeapAllocator>* make_nat(u64 size, u32 reserved = 0) {
  if (!reserved) {
    reserved = size + 4;
  }

  auto nat = bigmath::nat_new<HeapAllocator>(0ul, reserved);
  nat->places_count = size;

  for (u64 i = 0; i < size; ++i) {
    if (i % 7 == 0)
      nat->places[i] = ~0ul;
    else
      nat->places[i] = i;
  }

  return nat;
}

static void nat_add_word(benchmark::State& state) {
  const u64 words_count = 8;

  u64 words[words_count];
  for (u64 i = 0; i < words_count; ++i) {
    words[i] = (i % 4 == 0) ? ~0ul : i;
  }

  u64 offset = 0;
  auto a = make_nat(state.range(0));

  u32 places_count = a->places_count;

  for (auto _ : state) {
    for (u64 i = 0; i < words_count; ++i)
      a = bigmath::nat_add_word(a, words[(offset + i) & (words_count - 1)]);
    a->places_count = places_count;

    offset++;
  }

  bigmath::nat_free(a);
}

static void nat_add_nat(benchmark::State& state) {
  auto a = make_nat(state.range(0));
  auto b = make_nat(state.range(0));

  u32 places_count = a->places_count;

  for (auto _ : state) {
    a = bigmath::nat_add_nat(a, b);
    benchmark::DoNotOptimize(a->places[places_count - 1]);

    a->places_count = places_count;
  }

  bigmath::nat_free(a);
  bigmath::nat_free(b);
}

static void nat_add_nat_diff(benchmark::State& state) {
  const u64 base_size = 24;

  auto a = make_nat(base_size, base_size + state.range(0) + 4);
  auto b = make_nat(base_size + state.range(0));

  for (auto _ : state) {
    a = bigmath::nat_add_nat(a, b);
    benchmark::DoNotOptimize(a->places[a->places_count - 1]);

    a->places_count = base_size;
  }

  bigmath::nat_free(a);
  bigmath::nat_free(b);
}

BENCHMARK(nat_add_word)->Range(2, 50);
BENCHMARK(nat_add_nat)->Range(2, 2000);
BENCHMARK(nat_add_nat_diff)->Range(2, 2000);
BENCHMARK_MAIN();
