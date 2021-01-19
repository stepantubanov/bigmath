#include <bigmath/natural.h>

#include "helper.h"

TEST_CASE("nat square") {
  const u64 m = ~u64(0);

  REQUIRE(Z{0}.square() == Z{0, 0, 0, 0, 0, 0, 0, 0});
  REQUIRE(Z{1}.square() == Z{1, 0, 0, 0, 0, 0, 0, 0});
  REQUIRE(Z{3}.square() == Z{9, 0, 0, 0, 0, 0, 0, 0});
  REQUIRE(Z{1, 2, 3, 4}.square() == Z{1, 4, 10, 20, 25, 24, 16, 0});

  REQUIRE(Z{0, 0, 1, 0}.square() == Z{0, 0, 0, 0, 1});
  REQUIRE(Z{m, m, m, m}.square() == Z{1, 0, 0, 0, m - 1, m, m, m});
  REQUIRE(Z{0, m, 1, 0}.square() == Z{0, 0, 1, m - 3, 3, 0, 0, 0});
  REQUIRE(Z{0, m, m, 0}.square() == Z{0, 0, 1, 0, m - 1, m, 0, 0});

  REQUIRE(Z{0, 0, 1, m}.square() == Z{0, 0, 0, 0, 1, m - 1, 2, m - 1});

  SECTION("big number") {
    u32 big_size = 5 + rand() % 20;

    auto nat = bigmath::nat_new<HeapAllocator>({}, big_size);
    nat->places_count = big_size;
    for (u32 i = 0; i < bigmath::place_t::size_v * big_size; ++i) {
      nat->words[i] = 1;
    }

    bigmath::natural<HeapAllocator>* result = nullptr;
    result = bigmath::nat_square(result, nat);

    REQUIRE(result->places_count == 2 * big_size);
    for (u32 i = 0; i < bigmath::place_t::size_v * big_size; ++i) {
      REQUIRE(result->words[i] == (i + 1));
    }
  }
}
