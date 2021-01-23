#include <immintrin.h>
#include <string.h>

#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

void square_nat(place_t* _res, const place_t* _nat, u64 _nat_size) {
  auto res = reinterpret_cast<u64*>(_res);
  auto nat = reinterpret_cast<const u64*>(_nat);

  u32 nat_size = _nat_size * place_t::size_v;

  // Initialize by filling the array with squares.
  BIGMATH_LOOP_UNROLL_TWO
  for (u32 i = 0; i < nat_size; ++i) {
    u128 w = u128(nat[i]) * nat[i];
    res[i * 2 + 0] = u64(w);
    res[i * 2 + 1] = u64(w >> 64);
  }

  // Column-wise computation.
  {
    u128 pre_carry = 0;  // Before multiplying by 2
    u64 post_carry = 0;  // After multiplying by 2 and combining.

    auto nat_ptr = nat;
    auto res_ptr = res + 1;

    auto compute_column = [nat, &nat_ptr, &res_ptr, &pre_carry, &post_carry](
                              u32 spacing, u32 iterations) {
      u64 pre_digit = u64(pre_carry);
      pre_carry = (pre_carry >> 64);

      const u64* a = nat_ptr;
      const u64* b = nat_ptr + 1 + spacing;

      BIGMATH_LOOP_UNROLL_TWO
      for (; iterations > 0; --iterations) {
        u128 product = u128(a[0]) * b[0];
        product += pre_digit;
        pre_digit = u64(product);
        pre_carry += u64(product >> 64);

        --a;
        ++b;
      }

      u128 post_digit;
      post_digit = 2 * u128(pre_digit) + post_carry + res_ptr[0];

      res_ptr[0] = u64(post_digit);
      post_carry = u64(post_digit >> 64);
    };

    for (u32 i = 0; i < nat_size - 1; ++i) {
      compute_column(i & 1, i / 2 + 1);

      nat_ptr += (i & 1);
      res_ptr++;
    }

    for (u32 i = 1; i < nat_size; ++i) {
      compute_column(i & 1, nat_size / 2 - (i + 1) / 2);

      nat_ptr += (i & 1);
      res_ptr++;
    }

    res_ptr[0] += post_carry;
  }
}

}  // namespace internal
}  // namespace bigmath

#endif
