#pragma once

#include "internal.h"

namespace bigmath {

struct place_t {
  static constexpr u64 size = 4;
  u64 values[size];
};

struct raw_natural {
  u32 places_count;
  u32 places_capacity;  // always >= places_count

  union {
    place_t places[0];
    u64 words[0];
  };
};

template <typename Allocator>
struct natural : raw_natural {};

template <typename Allocator>
inline natural<Allocator>* nat_alloc(u32 places_capacity) {
  u64 size = sizeof(natural<Allocator>) + sizeof(place_t) * places_capacity;
  return static_cast<natural<Allocator>*>(Allocator::alloc(size));
}

template <typename Allocator>
inline natural<Allocator>* nat_ensure(natural<Allocator>* nat,
                                      u32 required_capacity) {
  u32 capacity = nat->places_capacity;
  if (capacity >= required_capacity) {
    return nat;
  }

  capacity = capacity * 3 / 2;
  if (capacity < required_capacity) {
    capacity = required_capacity;
  }

  nat = static_cast<natural<Allocator>*>(Allocator::realloc(
      nat, sizeof(natural<Allocator>) + sizeof(place_t) * capacity));
  nat->places_capacity = capacity;
  return nat;
}

template <typename Allocator>
inline void nat_free(natural<Allocator>* nat) {
  Allocator::free(nat);
}

template <typename Allocator>
inline natural<Allocator>* nat_new(place_t place, u32 places_capacity = 1) {
  auto nat = nat_alloc<Allocator>(places_capacity);

  nat->places_count = 1;
  nat->places_capacity = places_capacity;
  nat->places[0] = place;
  return nat;
}

template <typename Allocator>
inline natural<Allocator>* nat_new_words(const u64* words, u32 words_count,
                                         u32 places_capacity = 0) {
  u32 places_count = (words_count + place_t::size - 1) / place_t::size;
  if (places_capacity < places_count) {
    places_capacity = places_count;
  }

  auto nat = nat_alloc<Allocator>(places_capacity);

  nat->places_count = places_count;
  nat->places_capacity = places_capacity;
  for (u32 i = 0; i < words_count; ++i) {
    nat->words[i] = words[i];
  }
  for (u32 i = words_count; i < place_t::size * places_count; ++i) {
    nat->words[i] = 0;
  }

  return nat;
}

template <typename Allocator>
inline natural<Allocator>* nat_clone(natural<Allocator>* nat,
                                     u32 places_capacity = 0) {
  u32 count = nat->places_count;
  if (places_capacity < count) {
    places_capacity = count;
  }

  auto cloned = nat_alloc<Allocator>(places_capacity);
  cloned->places_count = count;
  cloned->places_capacity = places_capacity;
  for (u32 i = 0; i < count; ++i) {
    cloned->places[i] = nat->places[i];
  }
  return cloned;
}

// result += word
template <typename Allocator>
inline natural<Allocator>* nat_add_word(natural<Allocator>* result, u64 word) {
  result = nat_ensure<Allocator>(result, result->places_count + 1);

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

  result = nat_ensure(result, max_count + 1);

  result->places_count = internal::add_nat(result->places, result->places_count,
                                           other->places, other->places_count);
  return result;
}

}  // namespace bigmath
