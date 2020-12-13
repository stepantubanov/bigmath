#include "internal.h"

namespace bigmath {
namespace internal {

uint64_t add_word(uint64_t* nat, uint64_t nat_size, uint64_t word) {
  auto overflow = __builtin_uaddl_overflow(nat[0], word, &nat[0]);
  if (__builtin_expect(!overflow, 1)) {
    return nat_size;
  }

  for (uint64_t i = 1; i < nat_size; ++i) {
    overflow = __builtin_uaddl_overflow(nat[i], 1, &nat[i]);
    if (__builtin_expect(!overflow, 1)) {
      return nat_size;
    }
  }

  nat[nat_size] = 1;
  return nat_size + 1;
}
#include <stdio.h>

uint64_t add_nat(uint64_t* nat, uint64_t nat_size, const uint64_t* other,
                 uint64_t other_size) {
  uint64_t carry = 0;
  uint64_t nat_value;

  uint64_t min_size = nat_size < other_size ? nat_size : other_size;
  uint64_t max_size = nat_size < other_size ? other_size : nat_size;

  for (uint64_t i = 0; i < min_size; ++i) {
    uint64_t a = nat[i];
    uint64_t b = other[i];

    uint64_t c0 = __builtin_uaddl_overflow(a, carry, &a);
    uint64_t c1 = __builtin_uaddl_overflow(a, b, &nat[i]);

    carry = c0 + c1;
  }

  for (uint64_t i = min_size; i < max_size; ++i) {
    uint64_t a = i < nat_size ? nat[i] : 0;
    uint64_t b = i < other_size ? other[i] : 0;

    uint64_t c0 = __builtin_uaddl_overflow(a, carry, &a);
    uint64_t c1 = __builtin_uaddl_overflow(a, b, &nat[i]);

    carry = c0 + c1;
  }

  nat[max_size] = carry;
  return carry > 0 ? max_size + 1 : max_size;
}

}  // namespace internal
}  // namespace bigmath
