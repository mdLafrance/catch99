#define CATCH99_MAIN

#include "catch99.h"

TEST_CASE("Test tests") {
  CHECK(1 == 1);
  CHECK(2 == 2);
  REQUIRE(1 == 1);
}

TEST_CASE("Test Foo") {
  CHECK(1 == 1);

  if (1) {
    SKIP("Foo is not available")
  }

  CHECK(2 == 2);
  REQUIRE(1 == 3);
}
