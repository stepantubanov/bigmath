#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

u64 mul_word(void* _res, const void* _nat, u64 nat_size, u64 word) {
  u64* res = (u64*)_res;
  u64* nat = (u64*)_nat;

  __uint128_t carry = 0;
  for (u64 i = 0; i < 4 * nat_size; ++i) {
    __uint128_t m = nat[i];
    m *= word;

    __uint128_t v = carry;
    v += u64(m);
    carry = u64(carry >> 64);

    res[i] = u64(v);
    carry += u64(m >> 64);
  }

  if (!carry) {
    return nat_size;
  }

  res[4 * nat_size + 0] = u64(carry);
  res[4 * nat_size + 1] = 0;
  res[4 * nat_size + 2] = 0;
  res[4 * nat_size + 3] = 0;

  return nat_size + 1;
}

}  // namespace internal
}  // namespace bigmath

#endif
