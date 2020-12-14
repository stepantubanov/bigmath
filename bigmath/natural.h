#pragma once

#include "internal.h"

namespace bigmath {

struct raw_natural {
  u32 places_count;
  u32 places_capacity;  // always >= places_count + 2
  u64 places[0];
};

template <typename Allocator>
struct natural : raw_natural {};

struct place128_t {
  u64 p0, p1;
};

template <typename Allocator>
inline natural<Allocator>* nat_new(u64 value, u32 places_capacity = 1) {
  places_capacity += 2;
  u64 size = sizeof(natural<Allocator>) + sizeof(u64) * places_capacity;

  natural<Allocator>* nat =
      static_cast<natural<Allocator>*>(Allocator::alloc(size));

  nat->places_count = 1;
  nat->places_capacity = places_capacity;
  nat->places[0] = value;
  return nat;
}

template <typename Allocator>
inline natural<Allocator>* nat_new(place128_t places, u32 places_capacity = 2) {
  places_capacity += 2;

  u64 size = sizeof(natural<Allocator>) + sizeof(u64) * places_capacity;
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
inline natural<Allocator>* nat_clone(natural<Allocator>* nat) {
  u32 count = nat->places_count;

  auto cloned = nat_new<Allocator>(0, count);
  cloned->places_count = count;
  for (u32 i = 0; i < count; ++i) {
    cloned->places[i] = nat->places[i];
  }
  return cloned;
}

template <typename Allocator>
inline natural<Allocator>* nat_reserve(natural<Allocator>* nat,
                                       u32 required_capacity) {
  required_capacity += 2;

  u32 capacity = nat->places_capacity;
  if (capacity >= required_capacity) {
    return nat;
  }

  // next power of two
  capacity = 1u << (32u - __builtin_clz(required_capacity - 1));

  nat = static_cast<natural<Allocator>*>(Allocator::realloc(
      nat, sizeof(natural<Allocator>) + sizeof(u64) * capacity));
  nat->places_capacity = capacity;
  return nat;
}

// result += word
template <typename Allocator>
inline natural<Allocator>* nat_add_word(natural<Allocator>* result, u64 word) {
  result = nat_reserve<Allocator>(result, result->places_count + 1);

  result->places_count =
      internal::add_word(result->places, result->places_count, word);
  return result;
}

// result += other
template <typename Allocator>
inline natural<Allocator>* nat_add_nat(natural<Allocator>* result,
                                       const raw_natural* other) {
  u64 max_count = result->places_count > other->places_count
                      ? result->places_count
                      : other->places_count;

  result = nat_reserve(result, max_count + 1);

  result->places_count = internal::add_nat(result->places, result->places_count,
                                           other->places, other->places_count);
  return result;
}

}  // namespace bigmath
