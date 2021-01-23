#include <immintrin.h>
#include <string.h>

#include "internal.h"

#ifdef BIGMATH_BUILTIN_INT128

namespace bigmath {
namespace internal {

s32 compare_nat(const place_t* _a, const place_t* _b, u64 size) {
  const u64* a = reinterpret_cast<const u64*>(_a);
  const u64* b = reinterpret_cast<const u64*>(_b);

  size *= place_t::size_v;

  for (u32 i = 0; i < size; ++i) {
    s64 diff = s64(a[i]) - s64(b[i]);

    if (diff != 0) {
      return diff;
    }
  }

  return 0;
}

}  // namespace internal
}  // namespace bigmath

#endif
