#include "internal.h"

namespace bigmath {
namespace internal {

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
  u64 min_size, max_size;
  const u64* nat_max;

  if (nat_size < other_size) {
    min_size = nat_size;
    max_size = other_size;
    nat_max = other;
  } else {
    min_size = other_size;
    max_size = nat_size;
    nat_max = nat;
  }

  u64 i = 0;
  u64 carry = 0;

  for (; i < min_size; ++i) {
    u64 a = nat[i];
    u64 b = other[i];

    auto sum = __uint128_t(a);
    sum += b;
    sum += carry;

    nat[i] = u64(sum);
    carry = u64(sum >> 64);
  }

  if (carry) {
    for (;;) {
      if (__builtin_expect(i == max_size, 0)) {
        nat[i] = 1;
        return max_size + 1;
      }

      u64 v = nat_max[i] + 1;
      nat[i] = v;
      ++i;

      if (__builtin_expect(v != 0, 1)) {
        carry = 0;
        break;
      }
    }
  }

  for (; i < max_size; ++i) {
    nat[i] = nat_max[i];
  }

  nat[max_size] = carry;
  return carry > 0 ? max_size + 1 : max_size;
}

#endif

}  // namespace internal
}  // namespace bigmath
