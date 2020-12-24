#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

u64 add_word(void* _nat, u64 nat_size, u64 word) {
  u64* nat = (u64*)_nat;

  __uint128_t sum = __uint128_t(nat[0]) + word;
  nat[0] = u64(sum);

  if (__builtin_expect(u64(sum >> 64) == 0, 1)) {
    return nat_size;
  }

  u64 i = 1;
  for (; i < 4 * nat_size; ++i) {
    nat[i] += 1;
    if (__builtin_expect(nat[i] != 0, 1)) {
      return nat_size;
    }
  }

  nat[i + 0] = 1;
  nat[i + 1] = 0;
  nat[i + 2] = 0;
  nat[i + 3] = 0;
  return nat_size + 1;
}

u64 add_nat(void* _nat, u64 nat_size, const void* _other, u64 other_size) {
  u64* nat = (u64*)_nat;
  u64* other = (u64*)_other;

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

  for (; i < 4 * min_size; ++i) {
    auto sum = __uint128_t(nat[i]);
    sum += carry;
    sum += other[i];

    nat[i] = u64(sum);
    carry = u64(sum >> 64);
  }

  if (carry) {
    for (;;) {
      if (__builtin_expect(i == 4 * max_size, 0)) {
        nat[i + 0] = 1;
        nat[i + 1] = 0;
        nat[i + 2] = 0;
        nat[i + 3] = 0;
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

  if (nat != nat_max) {
    for (; i < 4 * max_size; ++i) {
      nat[i] = nat_max[i];
    }
  }

  return max_size;
}

}  // namespace internal
}  // namespace bigmath

#endif
