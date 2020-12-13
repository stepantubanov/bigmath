#pragma once

#include "inttypes.h"

namespace bigmath {
namespace internal {

u64 add_word(u64* nat, u64 nat_size, u64 word);
u64 add_nat(u64* nat, u64 nat_size, const u64* other, u64 other_size);

}  // namespace internal
}  // namespace bigmath
