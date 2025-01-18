#define CATCH99_CONFIG_MAIN

#include "catch99.h"

int foo() {
    return 5;
}

// Define another test case that fails
TEST_CASE("main - test 1  line 6") {
  int sum = -2 + -3;
  REQUIRE(sum == -5); // This will pass
}
