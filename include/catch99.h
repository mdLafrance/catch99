#ifndef CATCH99
#define CATCH99

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =================== Constants ======================== //

#ifndef CATCH99_MAX_TEST_CASES
#define CATCH99_MAX_TEST_CASES 512
#endif

#ifndef CATCH99_MAX_TESTS
#define CATCH99_MAX_TESTS 128
#endif

// =================== Utility Macros =================== //

#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define CONCAT(a, b) a##b

#define MAKE_FN_NAME(line) CONCAT(CNN_case_, line)
#define MAKE_REG_FN_NAME(line) CONCAT(CNN_register_case_, line)
#define CASE_NAME() MAKE_FN_NAME(__LINE__)
#define CASE_REGISTER_NAME() MAKE_REG_FN_NAME(__LINE__)

#ifdef CATCH99_NO_COLORS
#define CNN_TERM_RED ""
#define CNN_TERM_GREEN ""
#define CNN_TERM_GRAY ""
#define CNN_TERM_NC ""
#define CNN_TERM_UNDERLINE ""
#define CNN_TERM_BOLD ""
#else
#define CNN_TERM_RED "\033[31m"
#define CNN_TERM_GREEN "\033[32m"
#define CNN_TERM_GRAY "\033[90m"
#define CNN_TERM_NC "\033[0m"
#define CNN_TERM_UNDERLINE "\033[4m"
#define CNN_TERM_BOLD "\033[1m"
#endif

// =================== Data types ======================= //

typedef enum { REQUIRE, CHECK } TestType;

/// A single test. Remembers the invocation details and outcome for report
/// later.
typedef struct Test {
  size_t lineno;
  const char *text;
  char passed;
  TestType test_type;
} Test;

/// A single test case. A test case organizes multiple tests that are logically
/// related.
typedef struct TestCase {
  char failed;
  size_t lineno;
  const char *filename;
  const char *description;
  const char *case_fn_name;
  void (*case_fn)(void);
  size_t num_tests;
  Test tests[CATCH99_MAX_TESTS];
} TestCase;

// =================== Forward declares ================= //

static void _cnn_format_datetime_str(char *restrict out_str);
static void _cnn_format_elapsed_string(double dt, char *restrict out_str);
static void _cnn_register_test_with_case(Test t, const char *restrict file_name,
                                         const char *restrict fn_name);

// =================== Macros =========================== //
#define TEST_CASE(desc)                                                        \
  static void CASE_NAME()(void);                                               \
  __attribute__((constructor)) static void CASE_REGISTER_NAME()(void) {        \
    TestCase test_case = {.lineno = __LINE__,                                  \
                          .filename = __FILE__,                                \
                          .description = desc,                                 \
                          .case_fn_name = STRINGIFY(CASE_NAME()),              \
                          .case_fn = CASE_NAME()};                             \
                                                                               \
    if (_cnn_num_test_cases >= CATCH99_MAX_TEST_CASES) {                       \
      fprintf(stderr, "MAXIMUM TEST CASES EXCEEDED\n");                        \
      exit(1);                                                                 \
    }                                                                          \
                                                                               \
    _cnn_test_cases[_cnn_num_test_cases++] = test_case;                        \
  };                                                                           \
                                                                               \
  static void CASE_NAME()()

#define CHECK(p)                                                               \
  do {                                                                         \
    struct Test t = {                                                          \
        .lineno = __LINE__, .text = #p, .passed = p, .test_type = CHECK};      \
    _cnn_register_test_with_case(t, __FILE__, __func__);                       \
  } while (0);

#define REQUIRE(p)                                                             \
  do {                                                                         \
    struct Test t = {                                                          \
        .lineno = __LINE__, .text = #p, .passed = p, .test_type = REQUIRE};    \
    _cnn_register_test_with_case(t, __FILE__, __func__);                       \
                                                                               \
    if (!(p)) {                                                                \
      return;                                                                  \
    }                                                                          \
  } while (0);

// =================== Guarded impls ==================== //

#ifndef CATCH99_MAIN
extern TestCase _cnn_test_cases[CATCH99_MAX_TEST_CASES];
extern size_t _cnn_num_test_cases;
#else
TestCase _cnn_test_cases[CATCH99_MAX_TEST_CASES];
size_t _cnn_num_test_cases = 0;

static void _cnn_format_datetime_str(char dest[20]) {
  time_t t;
  struct tm *tm_info;
  time(&t);
  tm_info = localtime(&t);

  strftime(dest, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}

static void _cnn_format_elapsed_str(double dt, char out_str[20]) {
  int hours = (int)(dt / 3600);
  int minutes = (int)((dt - (hours * 3600)) / 60);
  int seconds = (int)(dt - (hours * 3600) - (minutes * 60));

  sprintf(out_str, "%d:%02d:%02d", hours, minutes, seconds);
}

static void _cnn_register_test_with_case(Test t, const char *restrict file_name,
                                         const char *restrict fn_name) {
  for (int i = 0; i < _cnn_num_test_cases; i++) {
    TestCase *__cnn_test_case = &_cnn_test_cases[i];
    if ((strcmp(__cnn_test_case->case_fn_name, fn_name) == 0) &&
        (strcmp(__cnn_test_case->filename, file_name) == 0)) {
      if (__cnn_test_case->num_tests >= CATCH99_MAX_TESTS) {
        fprintf(stderr, "MAXIMUM TESTS EXCEEDED FOR CASE %s\n",
                __cnn_test_case->description);
        exit(1);
      }
      __cnn_test_case->tests[__cnn_test_case->num_tests++] = t;
    }
  }
}

void run_tests() {
  clock_t t0 = clock();

  const int width = 40;
  char now_str[20];
  _cnn_format_datetime_str(now_str);
  printf("%s============================================================%s\n",
         CNN_TERM_GRAY, CNN_TERM_NC);
  printf("Catch99: Running %s%zu%s test cases           %s%s%s\n",
         CNN_TERM_UNDERLINE, _cnn_num_test_cases, CNN_TERM_NC, CNN_TERM_GRAY,
         now_str, CNN_TERM_NC);
  printf("%s============================================================%s\n",
         CNN_TERM_GRAY, CNN_TERM_NC);

  char tests_failed = 0;
  char test_cases_failed = 0;

  for (int i = 0; i < _cnn_num_test_cases; i++) {
    TestCase *current_case = &_cnn_test_cases[i];

    // Truncate description string
    int desc_len = strlen(current_case->description);
    char truncated_desc[width + 1];
    memset(truncated_desc, 0, width + 1);
    memcpy(truncated_desc, current_case->description,
           desc_len < width ? desc_len : width);
    truncated_desc[width] = '\0';

    // Print intro string for this case
    int diff = width - strlen(truncated_desc);
    printf("%s[%3d/%3zu]%s %s", CNN_TERM_GRAY, i + 1, _cnn_num_test_cases,
           CNN_TERM_NC, truncated_desc);
    for (int i = 0; i < diff; i++) {
      printf("%s.%s", CNN_TERM_GRAY, CNN_TERM_NC);
    }

    current_case->case_fn();

    for (int t = 0; t < current_case->num_tests; t++) {
      Test test = current_case->tests[t];

      if (test.passed) {
        printf("%s*%s", CNN_TERM_GREEN, CNN_TERM_NC);
      } else {
        ++tests_failed;

        // If this is the first test failed in this case, mark it down
        if (!current_case->failed) {
          ++test_cases_failed;
          current_case->failed = 1;
        }

        printf("%sE%s", CNN_TERM_RED, CNN_TERM_NC);
      }
    }

    printf("\n");
  }

  double dt = ((double)clock() - t0) / CLOCKS_PER_SEC;
  char elapsed_str[20];
  _cnn_format_elapsed_str(dt, elapsed_str);

  if (!tests_failed) {
    printf("\nAll tests %spassed %s%s%s\n", CNN_TERM_GREEN, CNN_TERM_GRAY,
           elapsed_str, CNN_TERM_NC);
  } else {
    printf(
        "%s-------------------------------------------------------------%s\n",
        CNN_TERM_GRAY, CNN_TERM_NC);

    for (int i = 0; i < _cnn_num_test_cases; i++) {
      TestCase *current_case = &_cnn_test_cases[i];

      if (!current_case->failed) {
        continue;
      }

      printf("\n%s%s%s\n", CNN_TERM_BOLD, current_case->description,
             CNN_TERM_NC);
      printf("%s%s:%zu%s\n\n", CNN_TERM_GRAY, current_case->filename,
             current_case->lineno, CNN_TERM_NC);

      for (int t = 0; t < current_case->num_tests; t++) {
        Test test = current_case->tests[t];

        if (!test.passed) {
          const char *test_type_str;
          switch (test.test_type) {
          case CHECK:
            test_type_str = "CHECK";
            break;
          case REQUIRE:
            test_type_str = "REQUIRE";
            break;
          }
          printf("%sFAILED:%s  %s(%s)\n", CNN_TERM_RED, CNN_TERM_NC,
                 test_type_str, test.text);
        }
      }

      printf("\n%s-------------------------------------------------------------"
             "%s\n",
             CNN_TERM_GRAY, CNN_TERM_NC);
    }

    printf("\n%d tests in %d cases %sfailed%s (%s elapsed)%s  \n", tests_failed,
           test_cases_failed, CNN_TERM_RED, CNN_TERM_GRAY, elapsed_str,
           CNN_TERM_NC);
  }
}

int main(void) {
  run_tests();
  return 0;
}
#endif // CNN_TEST_MAIN

#endif // CATCH99
