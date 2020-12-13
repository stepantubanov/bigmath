#include <bigmath/natural.h>

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
