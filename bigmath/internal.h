#pragma once

#include <stdint.h>

namespace bigmath {
namespace internal {

uint64_t add_word(uint64_t* nat, uint64_t nat_size, uint64_t word);
uint64_t add_nat(uint64_t* nat, uint64_t nat_size, const uint64_t* other,
                 uint64_t other_size);

}  // namespace internal
}  // namespace bigmath
