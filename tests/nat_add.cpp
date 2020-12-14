#include <bigmath/natural.h>

#include <sstream>
#include <string>

#include "catch2.h"

struct HeapAllocator {
  static inline void* alloc(uint64_t size) { return ::malloc(size); }
  static inline void* realloc(void* ptr, uint64_t size) {
    return ::realloc(ptr, size);
  }
  static inline void free(void* ptr) { ::free(ptr); }
};

TEST_CASE("natural addition") {
  const uint64_t max_place_value = 0xFFFF'FFFF'FFFF'FFFFlu;
  auto nat = bigmath::nat_new<HeapAllocator>(0);

  SECTION("add word") {
    // add zero does nothing
    nat = bigmath::nat_add_word(nat, 0);
    REQUIRE(nat->places_count == 1);
    REQUIRE(nat->places[0] == 0);

    // add a word without overflow
    nat = bigmath::nat_add_word(nat, 3);
    REQUIRE(nat->places_count == 1);
    REQUIRE(nat->places[0] == 3);

    // add a word with overflow
    nat = bigmath::nat_add_word(nat, max_place_value - 1);
    REQUIRE(nat->places_count == 2);
    REQUIRE(nat->places[0] == 1);
    REQUIRE(nat->places[1] == 1);
  }

  SECTION("add nat") {
    // TODO:
    //   + implement test cases listed below.
    //   + ensure asm, builting int128 & overflow pass all the tests.
    //   + improve benchmark coverage (small + big, big + small, etc).
    //   - run benchmarks on both Zen2 & Intel machines.
    //   - tool to compare benchmark results.
    //   + try a few optimizations & variations.
    //     + replacing loop with dec & jz.
    //     + comment out unroll-4 version of the min loop.

    auto other = bigmath::nat_new<HeapAllocator>({3, 1});

    // add with no overflow
    nat = bigmath::nat_add_nat(nat, other);
    REQUIRE(nat->places_count == 2);
    REQUIRE(nat->places[0] == 3);
    REQUIRE(nat->places[1] == 1);

    nat = bigmath::nat_add_nat(nat, other);
    REQUIRE(nat->places_count == 2);
    REQUIRE(nat->places[0] == 6);
    REQUIRE(nat->places[1] == 2);

    bigmath::nat_free(other);

    // add with overflow
    other = bigmath::nat_new<HeapAllocator>({max_place_value, max_place_value});

    nat = bigmath::nat_add_nat(nat, other);
    REQUIRE(nat->places_count == 3);
    REQUIRE(nat->places[0] == 5);
    REQUIRE(nat->places[1] == 2);
    REQUIRE(nat->places[2] == 1);

    bigmath::nat_free(other);
  }

  bigmath::nat_free(nat);
}

using natural_ptr = bigmath::natural<HeapAllocator>*;

struct Z {
  natural_ptr nat;

  explicit Z(natural_ptr n) : nat(n) {}

  Z(std::initializer_list<u64> places) {
    nat = bigmath::nat_new<HeapAllocator>(0, places.size());
    nat->places_count = places.size();
    std::copy(places.begin(), places.end(), nat->places);
  }
  Z(Z&& other) noexcept : nat(std::exchange(other.nat, nullptr)) {}

  ~Z() noexcept {
    if (nat) {
      bigmath::nat_free(nat);
    }
  }
};

struct Z_pair {
  Z a, b;
};

struct Z_triple {
  Z a, b, c;

  Z_triple(Z&& _a, Z&& _b, Z&& _c) noexcept
      : a(std::move(_a)), b(std::move(_b)), c(std::move(_c)) {}
  Z_triple(Z_triple&& other) noexcept
      : a(std::move(other.a)), b(std::move(other.b)), c(std::move(other.c)) {}
};

Z_pair operator+(Z&& a, Z&& b) { return {std::move(a), std::move(b)}; }
Z_triple operator==(Z_pair&& add, Z&& c) {
  return {std::move(add.a), std::move(add.b), std::move(c)};
};

template <>
struct Catch::StringMaker<natural_ptr> {
  static std::string convert(const natural_ptr& value) {
    std::ostringstream ss;

    ss << "Z{";
    for (u32 i = 0; i < value->places_count; ++i) {
      if (i != 0) {
        ss << ", ";
      }
      ss << value->places[i];
    }
    ss << "}";

    return ss.str();
  }
};

class nat_matcher : public Catch::MatcherBase<natural_ptr> {
 public:
  nat_matcher(const natural_ptr& _expected) : expected(_expected) {}

  bool match(const natural_ptr& value) const override {
    // FIXME: implement actual routine in bigmath.
    if (expected->places_count != value->places_count) {
      return false;
    }

    for (u32 i = 0; i < expected->places_count; ++i) {
      if (expected->places[i] != value->places[i]) {
        return false;
      }
    }

    return true;
  }

  std::string describe() const override {
    return "== " + Catch::StringMaker<natural_ptr>::convert(expected);
  }

 private:
  natural_ptr expected;
};

inline nat_matcher nat_equals(natural_ptr expected) { return {expected}; }

#define NAT_TEST_SETUP(...)                                           \
  static Z_triple triples[] = {__VA_ARGS__};                          \
  u32 triple_index =                                                  \
      GENERATE(range(0u, u32(sizeof(triples) / sizeof(triples[0])))); \
  const Z_triple& triple = triples[triple_index];                     \
  auto a = triple.a.nat;                                              \
  auto b = triple.b.nat;                                              \
  auto c = triple.c.nat;

TEST_CASE("nat add nat") {
  u64 m = ~u64(0);

  NAT_TEST_SETUP(Z{0} + Z{0} == Z{0},     //
                 Z{0} + Z{m} == Z{m},     //
                 Z{m} + Z{1} == Z{0, 1},  //
                 Z{m, 1} + Z{1, 1} == Z{0, 3},
                 Z{m, m, 1} + Z{m, m, 1} == Z{m - 1, m, 3},
                 Z{1, 2, 3, 4} + Z{1, 2, 3, 4} == Z{2, 4, 6, 8},
                 Z{1} + Z{1, 2, 3, 4} == Z{2, 2, 3, 4},
                 Z{m} + Z{2, 3, 4, 5} == Z{1, 4, 4, 5},
                 Z{1, 2, 3, m} + Z{1, 2, 3, 4, 5, 6} == Z{2, 4, 6, 3, 6, 6},
                 Z{1, 2, 3, 4, m} + Z{1, 2, 3, 4, m} == Z{2, 4, 6, 8, m - 1, 1},
                 Z{m, m, m, m, m} + Z{1, 2, 3, 4, 5} == Z{0, 2, 3, 4, 5, 1});

  SECTION("a + b == c") {
    a = bigmath::nat_clone(a);
    a = bigmath::nat_add_nat(a, b);

    REQUIRE_THAT(a, nat_equals(c));

    bigmath::nat_free(a);
  }

  SECTION("b + a == c") {
    b = bigmath::nat_clone(b);
    b = bigmath::nat_add_nat(b, a);

    REQUIRE_THAT(b, nat_equals(c));

    bigmath::nat_free(b);
  }
}
