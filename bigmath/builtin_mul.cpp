#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

u64 mul_word(void* _res, const void* _nat, u64 nat_size, u64 word) {
  u64* res = (u64*)_res;
  u64* nat = (u64*)_nat;

  u64 carry = 0;
  nat_size *= 4;

  for (u64 i = 0; i < nat_size; ++i) {
    __uint128_t t = __uint128_t(nat[i]) * word;
    t += carry;

    res[i] = u64(t);
    carry = u64(t >> 64);
  }

  if (!carry) {
    return 0;
  }

  res[nat_size + 0] = u64(carry);
  res[nat_size + 1] = 0;
  res[nat_size + 2] = 0;
  res[nat_size + 3] = 0;

  return 1;
}

u64 mul_nat(void* _res, const void* _nat, u64 nat_size, const void* _other,
            u64 other_size) {
  u64* res = (u64*)_res;
  u64* nat = (u64*)_nat;
  u64* other = (u64*)_other;

  for (u64 i = 0; i < 4 * nat_size; ++i) {
    res[i] = 0;
  }

  u64 carry;

  for (u64 j = 0; j < 4 * other_size; ++j) {
    u64 word = other[j];

    carry = 0;
    for (u64 i = 0; i < 4 * nat_size; ++i) {
      __uint128_t t = __uint128_t(nat[i]) * word;
      t += res[i + j];
      t += carry;

      res[i + j] = u64(t);
      carry = u64(t >> 64);
    }

    res[j + 4 * nat_size + 0] = carry;
  }

  // TODO: normalize the value by removing leading zeros
  u64 size = nat_size + other_size;
  return size;
}

/*
u64 mul_nat(void* _res, const void* _nat, u64 nat_size, const void* _other,
            u64 other_size) {
  u64* res = (u64*)_res;
  u64* nat = (u64*)_nat;
  u64* other = (u64*)_other;

  if (nat_size < other_size) {
    u64* t = nat;
    nat = other;
    other = t;

    u64 tt = nat_size;
    nat_size = other_size;
    other_size = tt;
  }

  nat_size *= 4;
  other_size *= 4;

  u64* other_end = other + other_size;

  __uint128_t carry = 0;
  u64 w = 0;

  for (u64 i = 0; i < nat_size; ++i) {
    for (u64 *a = nat + i, *b = other; a >= nat && b < other_end; --a, ++b) {
      __uint128_t t = __uint128_t(*a) * (*b);
      t += w;

      w = u64(t);
      carry += u64(t >> 64);
    }

    res[i] = w;

    w = u64(carry);
    carry = u64(carry >> 64);
  }

  for (u64 i = 1; i < other_size; ++i) {
    for (u64 *a = nat + nat_size - 1, *b = other + i; b < other_end; --a, ++b) {
      __uint128_t t = __uint128_t(*a) * (*b);
      t += w;

      w = u64(t);
      carry += u64(t >> 64);
    }

    res[nat_size + i - 1] = w;

    w = u64(carry);
    carry = u64(carry >> 64);
  }

  res[nat_size + other_size - 1] = w;
  return (nat_size + other_size) / 4;
}
*/

}  // namespace internal
}  // namespace bigmath

#endif
