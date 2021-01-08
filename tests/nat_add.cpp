#include <bigmath/natural.h>

#include "helper.h"

TEST_CASE("nat add") {
  const u64 m = ~u64(0);

  SECTION("add word") {
    SECTION("basic") {
      REQUIRE(Z{0} + 0 == Z{0});
      REQUIRE(Z{35} + 70 == Z{105});
      REQUIRE(Z{m} + 135 == Z{134, 1});

      REQUIRE(Z{m, m, m, m, m} + 1 == Z{0, 0, 0, 0, 0, 1});
    }

    SECTION("accumulating results") {
      Z r{m, m, m, m};
      r.nat = bigmath::nat_add_word(r.nat, 10);
      r.nat = bigmath::nat_add_word(r.nat, 20);
      r.nat = bigmath::nat_add_word(r.nat, 30);

      REQUIRE(r == Z{59, 0, 0, 0, 1});
    }
  }

  SECTION("add nat") {
    REQUIRE(Z{0} + Z{0} == Z{0});

    REQUIRE(Z{0} + Z{m} == Z{m});
    REQUIRE(Z{m} + Z{0} == Z{m});

    REQUIRE(Z{m} + Z{1} == Z{0, 1});
    REQUIRE(Z{1} + Z{m} == Z{0, 1});

    REQUIRE(Z{m, 1} + Z{1, 1} == Z{0, 3});
    REQUIRE(Z{1, 1} + Z{m, 1} == Z{0, 3});

    REQUIRE(Z{m, m, 1} + Z{m, m, 1} == Z{m - 1, m, 3});

    REQUIRE(Z{1, 2, 3, 4} + Z{1, 2, 3, 4} == Z{2, 4, 6, 8});
    REQUIRE(Z{1, 2, 3, m} + Z{1, 2, 3, 4} == Z{2, 4, 6, 3, 1});

    REQUIRE(Z{1} + Z{1, 2, 3, 4} == Z{2, 2, 3, 4});
    REQUIRE(Z{1, 2, 3, 4} + Z{1} == Z{2, 2, 3, 4});

    REQUIRE(Z{m} + Z{2, 3, 4, 5} == Z{1, 4, 4, 5});
    REQUIRE(Z{2, 3, 4, 5} + Z{m} == Z{1, 4, 4, 5});

    REQUIRE(Z{1, 2, 3, m} + Z{1, 2, 3, 4, 5, 6} == Z{2, 4, 6, 3, 6, 6});
    REQUIRE(Z{1, 2, 3, 4, 5, 6} + Z{1, 2, 3, m} == Z{2, 4, 6, 3, 6, 6});

    REQUIRE(Z{1, 2, 3, 4, m} + Z{1, 2, 3, 4, m} == Z{2, 4, 6, 8, m - 1, 1});

    REQUIRE(Z{m, m, m, m, m} + Z{1, 2, 3, 4, 5} == Z{0, 2, 3, 4, 5, 1});
    REQUIRE(Z{1, 2, 3, 4, 5} + Z{m, m, m, m, m} == Z{0, 2, 3, 4, 5, 1});
  }

  SECTION("add nat (little + big)") {
    auto nat = bigmath::nat_new<HeapAllocator>({}, 4);
    nat->places_count = 4;
    for (u32 i = 0; i < bigmath::place_t::size * 4; ++i) {
      nat->words[i] = i;
    }

    auto other = bigmath::nat_new<HeapAllocator>({}, 200);
    other->places_count = 200;
    for (u32 i = 0; i < bigmath::place_t::size * 200; ++i) {
      other->words[i] = i;
    }

    nat = bigmath::nat_add_nat(nat, other);

    REQUIRE(nat->places_count == 200);
    REQUIRE(nat->words[0] == 0);
    REQUIRE(nat->words[1] == 2);
    REQUIRE(nat->words[2] == 4);

    u32 last_index = 200 * bigmath::place_t::size - 1;
    REQUIRE(nat->words[last_index - 1] == last_index - 1);
    REQUIRE(nat->words[last_index] == last_index);

    bigmath::nat_free(nat);
    bigmath::nat_free(other);
  }

  SECTION("add nat (big + little)") {
    auto nat = bigmath::nat_new<HeapAllocator>({}, 200);
    nat->places_count = 200;
    for (u32 i = 0; i < bigmath::place_t::size * 200; ++i) {
      nat->words[i] = i;
    }

    auto other = bigmath::nat_new<HeapAllocator>({}, 2);
    other->places_count = 2;
    for (u32 i = 0; i < bigmath::place_t::size * 2; ++i) {
      other->words[i] = i;
    }

    nat = bigmath::nat_add_nat(nat, other);

    REQUIRE(nat->places_count == 200);
    REQUIRE(nat->words[0] == 0);
    REQUIRE(nat->words[1] == 2);
    REQUIRE(nat->words[2] == 4);

    u32 last_index = 200 * bigmath::place_t::size - 1;
    REQUIRE(nat->words[last_index - 1] == last_index - 1);
    REQUIRE(nat->words[last_index] == last_index);

    bigmath::nat_free(nat);
    bigmath::nat_free(other);
  }
}
