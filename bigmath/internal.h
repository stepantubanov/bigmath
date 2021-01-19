#pragma once

#include "inttypes.h"
#include "place.h"

#define BIGMATH_LIKELY(_condition_) __builtin_expect(_condition_, 1)
#define BIGMATH_UNLIKELY(_condition_) __builtin_expect(_condition_, 0)

#define BIGMATH_LOOP_UNROLL_TWO _Pragma("clang loop unroll_count(2)")

namespace bigmath {
namespace internal {

bool add_word(place_t* nat, u64 nat_size, u64 word);
bool add_nat(place_t* nat, u64 nat_size, const place_t* other, u64 other_size);

bool mul_word(place_t* res, const place_t* nat, u64 nat_size, u64 word);
void mul_nat(place_t* res, const place_t* nat, u64 nat_size,
             const place_t* other, u64 other_size);

void square_nat(place_t* res, const place_t* nat, u64 nat_size);

}  // namespace internal
}  // namespace bigmath
