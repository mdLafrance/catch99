#include "catch99.h"

TEST_CASE("Testing some string stuff"){REQUIRE("asdf" == "asdf")}


TEST_CASE("More tests line 6") {
  REQUIRE(3 == 3); // This will pass
  REQUIRE(3 == 3); // This will pass
}
