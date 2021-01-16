#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

bool mul_word(place_t* _res, const place_t* _nat, u64 nat_size, u64 word) {
  auto res = reinterpret_cast<u64*>(_res);
  auto nat = reinterpret_cast<const u64*>(_nat);

  u64 carry = 0;
  nat_size *= place_t::size_v;

  for (u64 i = 0; i < nat_size; ++i) {
    u128 t = u128(nat[i]) * word;
    t += carry;

    res[i] = u64(t);
    carry = u64(t >> 64);
  }

  if (!carry) {
    return false;
  }

  res[nat_size + 0] = u64(carry);
  res[nat_size + 1] = 0;
  res[nat_size + 2] = 0;
  res[nat_size + 3] = 0;

  return true;
}

void mul_nat(place_t* _res, const place_t* _nat, u64 nat_size,
             const place_t* _other, u64 other_size) {
  auto res = reinterpret_cast<u64*>(_res);
  auto nat = reinterpret_cast<const u64*>(_nat);
  auto other = reinterpret_cast<const u64*>(_other);

  nat_size *= place_t::size_v;
  other_size *= place_t::size_v;

  for (u64 i = 0; i < nat_size; ++i) {
    res[i] = 0;
  }

  u64 carry;

  for (u64 j = 0; j < other_size; ++j) {
    u64 word = other[j];

    carry = 0;
    for (u64 i = 0; i < nat_size; ++i) {
      u128 t = u128(nat[i]) * word;
      t += res[i + j];
      t += carry;

      res[i + j] = u64(t);
      carry = u64(t >> 64);
    }

    res[j + nat_size] = carry;
  }
}

}  // namespace internal
}  // namespace bigmath

#endif
