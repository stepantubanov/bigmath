#pragma once

#include "internal.h"

namespace bigmath {

struct raw_natural {
  uint32_t places_count;
  uint32_t places_capacity;
  uint64_t places[0];
};

template <typename Allocator>
struct natural : raw_natural {};

struct place128_t {
  uint64_t p0, p1;
};

template <typename Allocator>
inline natural<Allocator>* nat_new(uint64_t value,
                                   uint64_t places_capacity = 4) {
  uint64_t size =
      sizeof(natural<Allocator>) + sizeof(uint64_t) * places_capacity;
  natural<Allocator>* nat =
      static_cast<natural<Allocator>*>(Allocator::alloc(size));

  nat->places_count = 1;
  nat->places_capacity = places_capacity;
  nat->places[0] = value;
  return nat;
}

template <typename Allocator>
inline natural<Allocator>* nat_new(place128_t places,
                                   uint64_t places_capacity = 4) {
  uint64_t size =
      sizeof(natural<Allocator>) + sizeof(uint64_t) * places_capacity;
  natural<Allocator>* nat =
      static_cast<natural<Allocator>*>(Allocator::alloc(size));

  nat->places_count = 2;
  nat->places_capacity = places_capacity;
  nat->places[0] = places.p0;
  nat->places[1] = places.p1;
  return nat;
}

template <typename Allocator>
inline void nat_free(natural<Allocator>* nat) {
  Allocator::free(nat);
}

template <typename Allocator>
inline natural<Allocator>* nat_reserve(natural<Allocator>* nat,
                                       uint64_t places_capacity) {
  uint64_t capacity = nat->places_capacity;
  if (capacity >= places_capacity) {
    return nat;
  }

  // next power of two
  capacity = 1lu << (64lu - __builtin_clzl(capacity - 1));

  nat = static_cast<natural<Allocator>*>(Allocator::realloc(
      nat, sizeof(natural<Allocator>) + sizeof(uint64_t) * capacity));
  nat->places_capacity = capacity;
  return nat;
}

// result += word
template <typename Allocator>
inline natural<Allocator>* nat_add_word(natural<Allocator>* result,
                                        uint64_t word) {
  result = nat_reserve<Allocator>(result, result->places_count + 1);

  result->places_count =
      internal::add_word(result->places, result->places_count, word);
  return result;
}

// result += other
template <typename Allocator>
inline natural<Allocator>* nat_add_nat(natural<Allocator>* result,
                                       const raw_natural* other) {
  uint64_t max_count = result->places_count > other->places_count
                           ? result->places_count
                           : other->places_count;

  result = nat_reserve(result, max_count + 1);

  result->places_count = internal::add_nat(result->places, result->places_count,
                                           other->places, other->places_count);
  return result;
}

}  // namespace bigmath
