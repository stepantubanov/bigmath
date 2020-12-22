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
  u32 size0 = state.range(0);
  u32 size1 = state.range(1);
  u32 max_size = size0 > size1 ? size0 : size1;

  auto a = make_nat(size0, max_size + 4);
  auto b = make_nat(size1, max_size + 4);

  for (auto _ : state) {
    a = bigmath::nat_add_nat(a, b);
    benchmark::DoNotOptimize(a->places[a->places_count - 1]);

    a->places_count = size0;
  }

  bigmath::nat_free(a);
  bigmath::nat_free(b);
}

BENCHMARK(nat_add_word)->Arg(2)->Arg(100);
BENCHMARK(nat_add_nat)
    ->Args({2, 2})
    ->Args({32, 32})
    ->Args({500, 500})
    ->Args({2000, 2000})
    ->Args({2, 5})
    ->Args({2, 60})
    ->Args({100, 2000})
    ->Args({2000, 100});
BENCHMARK_MAIN();
