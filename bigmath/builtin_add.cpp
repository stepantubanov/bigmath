#include "internal.h"

namespace bigmath {
namespace internal {

#ifdef BIGMATH_BUILTIN_OVERFLOW

u64 add_word(u64* nat, u64 nat_size, u64 word) {
  auto overflow = __builtin_uaddl_overflow(nat[0], word, &nat[0]);
  if (__builtin_expect(!overflow, 1)) {
    return nat_size;
  }

  for (u64 i = 1; i < nat_size; ++i) {
    nat[i] += 1;
    if (__builtin_expect(nat[i] != 0, 1)) {
      return nat_size;
    }
  }

  nat[nat_size] = 1;
  return nat_size + 1;
}

u64 add_nat(u64* nat, u64 nat_size, const u64* other, u64 other_size) {
  u64 carry = 0;
  u64 nat_value;

  u64 min_size = nat_size < other_size ? nat_size : other_size;
  u64 max_size = nat_size < other_size ? other_size : nat_size;

  for (u64 i = 0; i < min_size; ++i) {
    u64 a = nat[i];
    u64 b = other[i];

    u64 c0 = __builtin_uaddl_overflow(a, carry, &a);
    u64 c1 = __builtin_uaddl_overflow(a, b, &nat[i]);

    carry = c0 + c1;
  }

  for (u64 i = min_size; i < max_size; ++i) {
    u64 a = i < nat_size ? nat[i] : 0;
    u64 b = i < other_size ? other[i] : 0;

    u64 c0 = __builtin_uaddl_overflow(a, carry, &a);
    u64 c1 = __builtin_uaddl_overflow(a, b, &nat[i]);

    carry = c0 + c1;
  }

  nat[max_size] = carry;
  return carry > 0 ? max_size + 1 : max_size;
}

#endif

#ifdef BIGMATH_BUILTIN_INT128

u64 add_word(u64* nat, u64 nat_size, u64 word) {
  __uint128_t sum = __uint128_t(nat[0]) + word;
  nat[0] = u64(sum);

  if (__builtin_expect(u64(sum >> 64) == 0, 1)) {
    return nat_size;
  }

  for (u64 i = 1; i < nat_size; ++i) {
    nat[i] += 1;
    if (__builtin_expect(nat[i] != 0, 1)) {
      return nat_size;
    }
  }

  nat[nat_size] = 1;
  return nat_size + 1;
}

u64 add_nat(u64* nat, u64 nat_size, const u64* other, u64 other_size) {
  u64 carry = 0;
  u64 nat_value;

  u64 min_size = nat_size < other_size ? nat_size : other_size;
  u64 max_size = nat_size < other_size ? other_size : nat_size;

  for (u64 i = 0; i < min_size; ++i) {
    u64 a = nat[i];
    u64 b = other[i];

    __uint128_t sum = __uint128_t(a) + b + carry;
    nat[i] = u64(sum);
    carry = u64(sum >> 64);
  }

  for (u64 i = min_size; i < max_size; ++i) {
    u64 a = i < nat_size ? nat[i] : 0;
    u64 b = i < other_size ? other[i] : 0;

    __uint128_t sum = __uint128_t(a) + b + carry;
    nat[i] = u64(sum);
    carry = u64(sum >> 64);
  }

  nat[max_size] = carry;
  return carry > 0 ? max_size + 1 : max_size;
}

#endif

}  // namespace internal
}  // namespace bigmath
