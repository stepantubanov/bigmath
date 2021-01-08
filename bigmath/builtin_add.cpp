#include <immintrin.h>

#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

u64 add_word(void* _nat, u64 nat_size, u64 word) {
  u64* nat = (u64*)_nat;

  __uint128_t sum = __uint128_t(nat[0]) + word;
  nat[0] = u64(sum);

  if (__builtin_expect(u64(sum >> 64) == 0, 1)) {
    return 0;
  }

  nat_size = 4 * nat_size;

  for (u32 i = 1; i < nat_size; ++i) {
    nat[i]++;
    if (__builtin_expect(nat[i] != 0, 1)) {
      return 0;
    }
  }

  nat[nat_size + 0] = 1;
  nat[nat_size + 1] = 0;
  nat[nat_size + 2] = 0;
  nat[nat_size + 3] = 0;
  return 1;
}

u64 add_nat(void* _nat, u64 nat_size, const void* _other, u64 other_size) {
  using ull = unsigned long long;

  u64* nat = (u64*)_nat;
  u64* other = (u64*)_other;
  u64* nat_max;

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

    if (__builtin_expect(i == max_size, 0)) {
      nat[j + 0] = 1;
      nat[j + 1] = 0;
      nat[j + 2] = 0;
      nat[j + 3] = 0;
      return 1;
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

  return 0;
}

}  // namespace internal
}  // namespace bigmath

#endif
