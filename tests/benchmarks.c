#include "catch99.h"

TEST_CASE("Testing division") {
  float x = 0;

  CHECK(4 / 2 == 2);

  BENCHMARK("Division") {
    RESULT(2 + 2);
    RESULT(2 + 3);
  }
}
