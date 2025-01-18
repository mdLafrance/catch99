#include "catch99.h"

TEST_CASE("Test some addition") {
    CHECK(1 + 1 == 2);
    CHECK(2 + 2 == 3);
    CHECK(3 + 3 == 7);
    CHECK(4 + 4 == 8);
    REQUIRE(2 * 2 == 4);
}
