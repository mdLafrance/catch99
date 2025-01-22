# Catch99
> A subset of Catch2 for C99

This library aims to reimplement some of the core dx of the excellent Catch2 unit testing framework for C99.

This is not a NASA-grade unit test framework - I was writing something else in C and just wanted to test with something that was familiar and had pretty colors.

![20250121_22h30m11s_grim](https://github.com/user-attachments/assets/7e79e87d-c98e-4f80-b922-587ce066aeb8)

## Usage
This is a **single header file** of C99. Just put it somewhere it can be included.

Like with Catch2, simply import the header and start using the macros. See the example [test files](./tests/).

> IMPORTANT: You must define `#CATCH99_MAIN` *exactly once* within your testing source code, *before* importing `catch99.h`.

```c
#define CATCH99_MAIN

#include "catch99.h"

#include <foo.h>

TEST_CASE("Tests sanity") {
  CHECK(1 == 1);
  CHECK(2 == 3); // Failed checks wont stop the case
  REQUIRE(1 == 3); // Failed requirements WILL stop the case
  CHECK(4 == 4); // (This wont run)
}

TEST_CASE("Tests Foo") {
  if (!FOO_IS_ENABLED) {
    SKIP("Foo is not enabled");
  }

  REQUIRE(Foo() == "foo")
}
```

That's it! In the TU with `CATCH99_MAIN` defined, a `main()` function is automatically generated that discovers, runs, and reports on all tests in the source. Your testing source then compiles to an executable that will run and report on all tests.

See the included [cmake](./CMakeLists.txt) file for a recipe on creating a test binary to run.

### Configuration
Catch99 will respect the following defines:

- `CATCH99_MAX_TEST_CASES` - the maximum number of test cases that can be defined. Defaults to `64`.
- `CATCH99_MAX_TESTS_PER_CASE` - the maximum number of tests that can be defined per test case. Defaults to `64`.
- `CATCH99_NO_COLORS` - if defined, no colors will be used in the output.
- `CATCH99_TERM_WIDTH` - if defined (as a number), catch will use this value as the terminal width. Otherwise, the terminal width will be queried at runtime with ioctl.

### A note about implementation
Some catch features are trickier to implement in C. You can do it more sanely in C++ because:
- c++ inlines dont require external linkage
- c++ has lambdas 

Without those, you have to rely on complicated macros. Maybe I'll turn this into something more substantial in the future, but for now a janky header file is enough for my use case.

### Planned
- [x] CHECK and REQUIRE
- [x] Skip clause
- [x] Pretty printing
- [ ] Trap stdout and stderr
- [ ] Case labels
- [ ] Setup/teardown hooks
- [ ] Throws clauses
- [ ] Benchmarking
- [ ] Legible warning when using macros incorrectly
