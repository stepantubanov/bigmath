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

bigmath::natural<HeapAllocator>* make_nat(u64 places_count,
                                          u64 places_reserved = 0) {
  if (places_reserved < places_count + 1) {
    places_reserved = places_count + 1;
  }

  auto nat = bigmath::nat_new<HeapAllocator>({0, 0, 0, 0}, places_reserved);
  nat->places_count = places_count;

  for (u64 i = 0; i < bigmath::place_t::size * places_count; ++i) {
    if (i % 7 == 0)
      nat->words[i] = ~0ul;
    else
      nat->words[i] = i;
  }

  return nat;
}

static void nat_add_word(benchmark::State& state) {
  const u64 words_count = 8;

  u64 words[words_count];
  for (u64 i = 0; i < words_count; ++i) {
    words[i] = (i % 4 == 0) ? ~0ul : i;
  }

  u32 places_count = state.range(0);
  auto a = make_nat(places_count);

  u64 offset = 0;
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

  auto a = make_nat(size0, max_size + 1);
  auto b = make_nat(size1, max_size + 1);

  for (auto _ : state) {
    a = bigmath::nat_add_nat(a, b);
    benchmark::DoNotOptimize(a->places[a->places_count - 1]);

    a->places_count = size0;
  }

  bigmath::nat_free(a);
  bigmath::nat_free(b);
}

static void nat_mul_word(benchmark::State& state) {
  u32 places_count = state.range(0);
  auto a = make_nat(places_count);
  auto r = make_nat(places_count + 1);

  const u64 words_count = 8;

  u64 words[words_count];
  for (u64 i = 0; i < words_count; ++i) {
    words[i] = (i % 4 == 0) ? ~0ul : i;
  }

  u64 offset = 0;
  for (auto _ : state) {
    for (u64 i = 0; i < words_count; ++i)
      r = bigmath::nat_mul_word(r, a, words[(offset + i) & (words_count - 1)]);

    benchmark::DoNotOptimize(r->places[r->places_count - 1]);
    offset++;
  }
}

static void nat_mul_nat(benchmark::State& state) {
  auto a = make_nat(state.range(0));
  auto b = make_nat(state.range(1));
  auto r = make_nat(state.range(0) + state.range(1));

  for (auto _ : state) {
    r = bigmath::nat_mul_nat(r, a, b);
    benchmark::DoNotOptimize(r->places[r->places_count - 1]);
  }

  bigmath::nat_free(a);
  bigmath::nat_free(b);
}

BENCHMARK(nat_add_word)->Arg(2)->Arg(100);
BENCHMARK(nat_add_nat)
    ->Args({1, 1})
    ->Args({8, 8})
    ->Args({125, 125})
    ->Args({500, 500})
    ->Args({1, 5})
    ->Args({2, 60})
    ->Args({30, 500})
    ->Args({500, 30});
BENCHMARK(nat_mul_word)->Arg(2)->Arg(100);
BENCHMARK(nat_mul_nat)
    ->Args({1, 1})
    ->Args({4, 4})
    ->Args({15, 10})
    ->Args({32, 32});  // 1 KiB x 1 KiB
BENCHMARK_MAIN();
