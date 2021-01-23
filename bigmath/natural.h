#pragma once

#include "internal.h"

namespace bigmath {

struct raw_natural {
  // Each place is 32 bytes.
  // 1 << 22 places is ~134 megabytes.
  static constexpr u32 max_capacity_v = (1 << 20);

  // Places count (memory area beyond specified places count contains
  // uninitialized values). Must be non-zero.
  u32 places_count;

  // Allocated memory size (count of places allocated). Must be greater or equal
  // to places_count.
  u32 places_capacity;

  // Padding, to ensure "places" alignment on 16 bytes.
  u64 _padding;

  union {
    place_t places[0];
    u64 words[0];
  };
};

// TODO: Rename "Allocator" to "Harness"?
template <typename Allocator>
struct natural : raw_natural {};

template <typename Allocator>
inline auto nat_alloc(u32 places_capacity) {
  if (BIGMATH_UNLIKELY(places_capacity > raw_natural::max_capacity_v)) {
    Allocator::raise_alloc_error(places_capacity);
  }

  u32 size = sizeof(natural<Allocator>) + sizeof(place_t) * places_capacity;
  return static_cast<natural<Allocator>*>(Allocator::alloc(size));
}

template <typename Allocator>
inline auto nat_ensure(natural<Allocator>* nat, u32 required_capacity) {
  u32 current_capacity = nat ? nat->places_capacity : 0;
  if (current_capacity >= required_capacity) {
    return nat;
  }

  u32 new_capacity = current_capacity * 3 / 2;
  if (new_capacity < required_capacity) {
    new_capacity = required_capacity;
  }

  if (BIGMATH_UNLIKELY(new_capacity > raw_natural::max_capacity_v)) {
    Allocator::raise_alloc_error(new_capacity);
  }

  u32 size = sizeof(natural<Allocator>) + sizeof(place_t) * new_capacity;

  nat = static_cast<natural<Allocator>*>(Allocator::realloc(nat, size));
  nat->places_capacity = new_capacity;
  return nat;
}

template <typename Allocator>
inline void nat_free(natural<Allocator>* nat) {
  Allocator::free(nat);
}

template <typename Allocator>
inline auto nat_new(place_t place, u32 places_capacity = 1) {
  auto nat = nat_alloc<Allocator>(places_capacity);

  nat->places_count = 1;
  nat->places_capacity = places_capacity;
  nat->places[0] = place;
  return nat;
}

template <typename Allocator>
inline auto nat_new_words(const u64* words, u32 words_count,
                          u32 places_capacity = 0) {
  u32 places_count = (words_count + place_t::size_v - 1) / place_t::size_v;
  if (places_capacity < places_count) {
    places_capacity = places_count;
  }

  auto nat = nat_alloc<Allocator>(places_capacity);

  nat->places_count = places_count;
  nat->places_capacity = places_capacity;
  for (u32 i = 0; i < words_count; ++i) {
    nat->words[i] = words[i];
  }
  for (u32 i = words_count; i < place_t::size_v * places_count; ++i) {
    nat->words[i] = 0;
  }

  return nat;
}

template <typename Allocator>
inline auto nat_clone(natural<Allocator>* nat, u32 places_capacity = 0) {
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
inline auto nat_add_word(natural<Allocator>* result, u64 word) {
  u32 places_count = result->places_count;
  result = nat_ensure<Allocator>(result, places_count + 1);

  bool place_added = internal::add_word(result->places, places_count, word);

  if (BIGMATH_UNLIKELY(place_added)) {
    result->places_count = places_count + 1;
  }
  return result;
}

// result += other
template <typename Allocator>
inline auto nat_add_nat(natural<Allocator>* result, const raw_natural* other) {
  u32 old_count = result->places_count;
  u32 new_count = old_count;

  if (old_count < other->places_count) {
    new_count = other->places_count;
  }

  result = nat_ensure(result, new_count + 1);
  result->places_count = new_count;

  bool place_added = internal::add_nat(result->places, old_count, other->places,
                                       other->places_count);
  if (BIGMATH_UNLIKELY(place_added)) {
    result->places_count = new_count + 1;
  }
  return result;
}

// result - may be nullptr
// result = nat * word
template <typename Allocator>
inline auto nat_mul_word(natural<Allocator>* result, const raw_natural* nat,
                         u64 word) {
  u32 places_count = nat->places_count;
  result = nat_ensure(result, places_count + 1);
  result->places_count = places_count;

  bool place_added =
      internal::mul_word(result->places, nat->places, places_count, word);

  if (place_added) {
    result->places_count = places_count + 1;
  }
  return result;
}

template <typename Allocator>
inline auto nat_mul_nat(natural<Allocator>* result, const raw_natural* a,
                        const raw_natural* b) {
  u32 places_count = a->places_count + b->places_count;

  result = nat_ensure(result, places_count);
  result->places_count = places_count;

  // TODO: normalize (remove leading zeros)
  internal::mul_nat(result->places, a->places, a->places_count, b->places,
                    b->places_count);
  return result;
}

template <typename Allocator>
inline auto nat_square(natural<Allocator>* result, const raw_natural* nat) {
  u32 places_count = 2 * nat->places_count;

  result = nat_ensure(result, places_count);
  result->places_count = places_count;

  internal::square_nat(result->places, nat->places, nat->places_count);
  return result;
}

template <typename Allocator, typename TempAllocator>
inline auto nat_pow(natural<Allocator>* result, const raw_natural* a,
                    u32 power) {
  return nullptr;
}

template <typename Allocator, typename TempAllocator, u64 Threshold = 4>
inline auto nat_mul_nat_karatsuba(natural<Allocator>* result,
                                  const raw_natural* a, const raw_natural* b) {
  u32 places_count = a->places_count + b->places_count;
  result = nat_ensure(result, places_count);

  u8* buffer = TempAllocator::alloc();
  // internal::mul_karatsuba(buffer, result, a->places, a->places_count,
  // b->places,
  //                          b->places_count);

  return nullptr;
}

inline auto nat_compare(const raw_natural* a, const raw_natural* b) -> s32 {
  // TODO: compare sizes?
  u32 min_size =
      (a->places_count < b->places_count) ? a->places_count : b->places_count;

  return internal::compare_nat(a->places, b->places, min_size);
}

template <typename TempAllocator>
inline auto nat_to_text(const raw_natural* a, u32 radix) {
  return nullptr;
}

template <typename Allocator>
inline auto nat_from_text(u32 radix, const char* text, u32 size) {
  return nullptr;
}

}  // namespace bigmath
