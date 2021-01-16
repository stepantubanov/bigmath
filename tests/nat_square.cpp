#include <bigmath/natural.h>

#include "helper.h"

TEST_CASE("nat square") {
  const u64 m = ~u64(0);

  REQUIRE(Z{0}.square() == Z{0, 0, 0, 0, 0, 0, 0, 0});
  REQUIRE(Z{1}.square() == Z{1, 0, 0, 0, 0, 0, 0, 0});
  REQUIRE(Z{3}.square() == Z{9, 0, 0, 0, 0, 0, 0, 0});

  REQUIRE(Z{0, 0, 1, 0}.square() == Z{0, 0, 0, 0, 1});
  REQUIRE(Z{m, m, m, m}.square() == Z{1, 0, 0, 0, m - 1, m, m, m});
}
