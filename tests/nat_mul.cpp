#include <bigmath/natural.h>

#include "helper.h"

TEST_CASE("nat mul") {
  const u64 m = ~u64(0);

  SECTION("mul word") {
    REQUIRE(Z{0} * 3 == Z{0});
    REQUIRE(Z{3} * 0 == Z{0});
    REQUIRE(Z{3} * 5 == Z{15});
    REQUIRE(Z{3, 4, 5, 6, 7} * 10 == Z{30, 40, 50, 60, 70});

    REQUIRE(Z{0, 0, m, 1} * m == Z{0, 0, 1, m - 2, 1});
    REQUIRE(Z{0, 0, m, 1, 0, 0, 0, 0, 1, 0, 0, 0} * m ==
            Z{0, 0, 1, m - 2, 1, 0, 0, 0, m, 0, 0, 0});

    REQUIRE(Z{m, m, m, m} * 2 == Z{m - 1, m, m, m, 1});
    REQUIRE(Z{4, 0, 0, 4} * m == Z{m - 3, 3, 0, m - 3, 3});
    REQUIRE(Z{m, m, m, m} * m == Z{1, m, m, m, m - 1});
  }

  SECTION("mul nat") {
    REQUIRE(Z{0} * Z{0} == Z{0, 0, 0, 0, 0, 0, 0, 0});
    REQUIRE(Z{1} * Z{0} == Z{0, 0, 0, 0, 0, 0, 0, 0});
    REQUIRE(Z{2} * Z{1} == Z{2, 0, 0, 0, 0, 0, 0, 0});

    REQUIRE(Z{m, m, m, m} * Z{2} == Z{m - 1, m, m, m, 1});
    REQUIRE(Z{1, 2, 3, 4} * Z{2, 3} == Z{2, 7, 12, 17, 12});

    REQUIRE(Z{m, m, m, m} * Z{1, 1, 1, 1} ==
            Z{m, m - 1, m - 1, m - 1, 0, 1, 1, 1});

    REQUIRE(Z{m, m, m, m} * Z{m, m, m, m} == Z{1, 0, 0, 0, m - 1, m, m, m});
    REQUIRE(Z{m, m, m, m, m, m} * Z{m, 0, m, m} ==
            Z{1, m, 0, 0, m, m, m - 1, 0, m, m});

    // shift left 192 + 5 bits
    REQUIRE(Z{0, 0, 0, 32} * Z{m, 0, m, m, 1, 2, 3, 4} ==
            Z{0, 0, 0, m - 31, 31, m - 31, m, 63, 64, 96, 128});
  }

  SECTION("mul nat (little + big, big + little)") {
    u32 little_size = 1 + rand() % 5;
    u32 big_size = 5 + rand() % 20;

    u32 shift = rand() % (bigmath::place_t::size_v * little_size);

    auto nat = bigmath::nat_new<HeapAllocator>({}, little_size);
    nat->places_count = little_size;
    for (u32 i = 0; i < bigmath::place_t::size_v * little_size; ++i) {
      nat->words[i] = 0;
    }
    nat->words[shift] = 1;

    auto other = bigmath::nat_new<HeapAllocator>({}, big_size);
    other->places_count = big_size;
    for (u32 i = 0; i < bigmath::place_t::size_v * big_size; ++i) {
      other->words[i] = i;
    }

    bigmath::natural<HeapAllocator>* result = nullptr;

    // a * b
    result = bigmath::nat_mul_nat(result, nat, other);

    REQUIRE(result->places_count == little_size + big_size);
    for (u32 i = 0; i < shift; ++i) {
      REQUIRE(result->words[i] == 0);
    }
    for (u32 i = 0; i < big_size; ++i) {
      REQUIRE(result->words[i + shift] == i);
    }

    // b * a
    result = bigmath::nat_mul_nat(result, other, nat);

    REQUIRE(result->places_count == little_size + big_size);
    for (u32 i = 0; i < shift; ++i) {
      REQUIRE(result->words[i] == 0);
    }
    for (u32 i = 0; i < big_size; ++i) {
      REQUIRE(result->words[i + shift] == i);
    }
  }
}
