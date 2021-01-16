#include <immintrin.h>

#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

void square_nat(place_t* _res, const place_t* _nat, u64 nat_size) {
  auto res = reinterpret_cast<u64*>(_res);
  auto nat = reinterpret_cast<const u64*>(_nat);
  nat_size *= place_t::size_v;

  for (u64 i = 0; i < 2 * nat_size; ++i) {
    res[i] = 0;
  }

  for (u64 i = 0; i < nat_size; ++i) {
    u64 nat_i = nat[i];

    u128 w = res[i * 2] + u128(nat_i) * nat_i;

    res[i * 2] = u64(w);
    u128 carry = u64(w >> 64);

    for (u64 j = i + 1; j < nat_size; ++j) {
      w = u128(nat_i) * nat[j];

      u64 hi = u64(w >> 64);

      w = u64(w);
      w = 2 * w + res[i + j] + carry;

      res[i + j] = u64(w);

      carry = u64(w >> 64);
      carry += u128(hi) << 1;
    }

    carry += res[i + nat_size];

    res[i + nat_size] = u64(carry);
    if (i + 1 < nat_size) {
      res[i + 1 + nat_size] += u64(carry >> 64);
    }
  }
}

}  // namespace internal
}  // namespace bigmath

#endif
