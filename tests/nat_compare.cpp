#include <bigmath/natural.h>

#include "helper.h"

TEST_CASE("nat compare") {
  REQUIRE(Z{0} < Z{1});
  REQUIRE(Z{1, 2, 3, 4} == Z{1, 2, 3, 4});
  REQUIRE(Z{1, 2, 4, 5} < Z{1, 2, 4, 6});
  REQUIRE(Z{5, 2, 4, 5} > Z{1, 5, 4, 6});
  REQUIRE(Z{1, 1, 1, 1, 5, 5, 6, 8} > Z{1, 1, 1, 1, 5, 5, 6, 7});
  REQUIRE(Z{1, 1, 1, 1, 5, 5, 6, 7} == Z{1, 1, 1, 1, 5, 5, 6, 7});

  SECTION("big number") {
    u32 big_size = 10 + rand() % 20;

    auto a = bigmath::nat_new<HeapAllocator>({}, big_size);
    a->places_count = big_size;
    for (u32 i = 0; i < bigmath::place_t::size_v * big_size; ++i) {
      a->words[i] = i;
    }

    auto b = bigmath::nat_new<HeapAllocator>({}, big_size);
    b->places_count = big_size;
    for (u32 i = 0; i < bigmath::place_t::size_v * big_size; ++i) {
      b->words[i] = i;
    }
    b->words[b->places_count - 1] += 1;

    REQUIRE(bigmath::nat_compare(a, a) == 0);
    REQUIRE(bigmath::nat_compare(a, b) < 0);
    REQUIRE(bigmath::nat_compare(b, a) > 0);
  }
}
