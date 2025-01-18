# Catch99
> A subset of Catch2 for C99

This project aims to reimplement some core features of the excellent Catch2 unit testing framework for C99 - because I was writing something else in C and was missing having Catch.

## Usage
This is a **single header file** of C99 with no dependencies - just put it somewhere it can be included.

Like with Catch2, simply import the header and start using the macros.

> IMPORTANT: You must define `#CATCH99_MAIN` *exactly once* within your testing source code, *before* importing `catch99.h`.

```c
#define CATCH99_MAIN

#include "catch99.h"

TEST_CASE("Test tests") {
  CHECK(1 == 1);
  CHECK(2 == 2);
  REQUIRE(1 == 3);
}
```

That's it! In the TU with `CATCH99_MAIN` defined, a `main()` function is automatically generated that discovers, runs, and reports on all tests in the source.  

See the included [cmake](./CMakeLists.txt) file for a recipe on creating a test binary to run.

### A note about implementation
Single header libraries just dont make sense to implement in C. You can do more easily in C++ because:
- c++ inlines dont require external linkage
- c++ has lambdas (this is more directly related to implementing something like Catch2)

Without those, you have to rely on complicated macros - not something conducive to writing something complex. 

Maybe I'll turn this into something more substantial in the future, but for now this is enough for my use case.

### TODO
- [x] CHECK and REQUIRE
- [ ] Skip clause
- [ ] Throws clauses
- [ ] Benchmarking
