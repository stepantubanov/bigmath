#include <immintrin.h>

#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

bool add_word(place_t* _nat, u64 nat_size, u64 word) {
  auto nat = reinterpret_cast<u64*>(_nat);

  bool carry = __builtin_uaddl_overflow(nat[0], word, &nat[0]);
  if (BIGMATH_LIKELY(!carry)) {
    return false;
  }

  nat_size *= place_t::size_v;

  for (u32 i = 1; i < nat_size; ++i) {
    carry = __builtin_uaddl_overflow(nat[i], 1, &nat[i]);
    if (BIGMATH_LIKELY(!carry)) {
      return false;
    }
  }

  nat[nat_size + 0] = 1;
  nat[nat_size + 1] = 0;
  nat[nat_size + 2] = 0;
  nat[nat_size + 3] = 0;
  return true;
}

bool add_nat(place_t* _nat, u64 nat_size, const place_t* _other,
             u64 other_size) {
  using ull = unsigned long long;

  auto nat = reinterpret_cast<u64*>(_nat);
  auto other = reinterpret_cast<const u64*>(_other);

  const u64* nat_max;

  // TODO: assert(place_t::size_v == 4)

  u32 min_size, max_size;

  if (nat_size < other_size) {
    min_size = nat_size;
    max_size = other_size;
    nat_max = other;
  } else {
    min_size = other_size;
    max_size = nat_size;
    nat_max = nat;
  }

  u32 i = 0;
  bool carry = false;

  do {
    u32 j = 4 * i;

    carry = _addcarry_u64(carry, nat[j + 0], other[j + 0], (ull*)&nat[j + 0]);
    carry = _addcarry_u64(carry, nat[j + 1], other[j + 1], (ull*)&nat[j + 1]);
    carry = _addcarry_u64(carry, nat[j + 2], other[j + 2], (ull*)&nat[j + 2]);
    carry = _addcarry_u64(carry, nat[j + 3], other[j + 3], (ull*)&nat[j + 3]);

    ++i;
  } while (i < min_size);

  while (carry) {
    auto j = i * 4;

    if (BIGMATH_UNLIKELY(i == max_size)) {
      nat[j + 0] = 1;
      nat[j + 1] = 0;
      nat[j + 2] = 0;
      nat[j + 3] = 0;
      return true;
    }

    bool c;
    c = __builtin_uaddl_overflow(1, nat_max[j + 0], &nat[j + 0]);
    c = _addcarry_u64(c, nat_max[j + 1], 0, (ull*)&nat[j + 1]);
    c = _addcarry_u64(c, nat_max[j + 2], 0, (ull*)&nat[j + 2]);
    c = _addcarry_u64(c, nat_max[j + 3], 0, (ull*)&nat[j + 3]);
    ++i;

    carry = c;
  }

  if (nat_max == other) {
    for (u32 j = 4 * i, j_end = 4 * max_size; j < j_end; ++j) {
      nat[j] = other[j];
    }
  }

  return false;
}

}  // namespace internal
}  // namespace bigmath

#endif
