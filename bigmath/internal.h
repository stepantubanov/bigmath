#pragma once

#include "inttypes.h"

namespace bigmath {
namespace internal {

// TODO: use place_t

u64 add_word(void* nat, u64 nat_size, u64 word);
u64 add_nat(void* nat, u64 nat_size, const void* other, u64 other_size);
u64 mul_word(void* res, const void* nat, u64 nat_size, u64 word);

}  // namespace internal
}  // namespace bigmath
