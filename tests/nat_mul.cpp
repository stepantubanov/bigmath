#include <bigmath/natural.h>

#include "helper.h"

TEST_CASE("nat mul") {
  const u64 m = ~u64(0);

  SECTION("mul word") {
    REQUIRE(Z{0} * 3 == Z{0});
    REQUIRE(Z{3} * 0 == Z{0});
    REQUIRE(Z{3} * 5 == Z{15});
    REQUIRE(Z{3, 4, 5, 6, 7} * 10 == Z{30, 40, 50, 60, 70});

    REQUIRE(Z{m, m, m, m} * 2 == Z{m - 1, m, m, m, 1});
    REQUIRE(Z{4, 0, 0, 4} * m == Z{m - 3, 3, 0, m - 3, 3});
  }
}
