/*
 * MIT License
 *
 * Copyright (c) 2025 Maxime Lafrance
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CATCH99
#define CATCH99

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef CATCH99_MAX_TEST_CASES
#define CATCH99_MAX_TEST_CASES 512
#endif

#ifndef CATCH99_MAX_TESTS
#define CATCH99_MAX_TESTS 128
#endif

#define CNN__STR(x) #x
#define CNN__STRINGIFY(x) CNN__STR(x)
#define CNN__CONCAT(a, b) a##b

#define CNN__MAKE_FN_NAME(line) CNN__CONCAT(CNN__case_, line)
#define CNN__MAKE_REG_FN_NAME(line) CNN__CONCAT(CNN__register_case_, line)
#define CNN__CASE_NAME() CNN__MAKE_FN_NAME(__LINE__)
#define CNN__REGISTER_CASE_FN() CNN__MAKE_REG_FN_NAME(__LINE__)

#ifdef CATCH99_NO_COLORS
#define CNN__TERM_RED ""
#define CNN__TERM_GREEN ""
#define CNN__TERM_GRAY ""
#define CNN__TERM_NC ""
#define CNN__TERM_UNDERLINE ""
#define CNN__TERM_BOLD ""
#define CNN__TERM_STRIKE ""
#else
#define CNN__TERM_RED "\033[31m"
#define CNN__TERM_GREEN "\033[32m"
#define CNN__TERM_GRAY "\033[90m"
#define CNN__TERM_NC "\033[0m"
#define CNN__TERM_UNDERLINE "\033[4m"
#define CNN__TERM_BOLD "\033[1m"
#define CNN__TERM_STRIKE "\e[9m"
#endif

typedef enum { REQUIRE, CHECK } CNN__TestType;
typedef enum { PASSED, FAILED, SKIPPED } CNN__CaseStatus;

/// A single test. Remembers the invocation details and outcome for report
/// later.
typedef struct CNN__Test {
  uint32_t lineno;
  const char *text;
  char passed;
  CNN__TestType test_type;
} CNN__Test;

/// A single test case. A test case organizes multiple tests that are logically
/// related.
typedef struct CNN__TestCase {
  CNN__CaseStatus status;
  uint32_t lineno;
  const char *skip_message;
  const char *filename;
  const char *description;
  const char *case_fn_name;
  void (*case_fn)(void);
  size_t num_tests;
  CNN__Test tests[CATCH99_MAX_TESTS];
} CNN__TestCase;

typedef struct CNN__Context {
  uint32_t num_cases;
  uint32_t num_tests;
  uint32_t cases_failed;
  uint32_t cases_skipped;
  CNN__TestCase test_cases[CATCH99_MAX_TEST_CASES];
} CNN__Context;

// =================== Forward declares ================= //

CNN__Context *_cnn_get_context();
static void _cnn_format_datetime_str(char *out_str);
static void _cnn_format_elapsed_string(double dt, char *out_str);
static CNN__TestCase *_cnn_get_current_case(const char *filename,
                                            const char *fn_name);
void _cnn_register_case(CNN__TestCase test_case);
void _cnn_register_test_with_case(CNN__Test t, const char *file_name,
                                  const char *fn_name);
static int _cnn_get_term_width();

// =================== Macros =========================== //
#define TEST_CASE(desc)                                                        \
  static void CNN__CASE_NAME()(void);                                          \
  __attribute__((constructor)) static void CNN__REGISTER_CASE_FN()(void) {     \
    CNN__TestCase test_case = {.lineno = __LINE__,                             \
                               .filename = __FILE__,                           \
                               .description = desc,                            \
                               .case_fn_name =                                 \
                                   CNN__STRINGIFY(CNN__CASE_NAME()),           \
                               .case_fn = CNN__CASE_NAME()};                   \
                                                                               \
    _cnn_register_case(test_case);                                             \
  };                                                                           \
                                                                               \
  static void CNN__CASE_NAME()()

#define CHECK(p)                                                               \
  do {                                                                         \
    struct CNN__Test t = {                                                     \
        .lineno = __LINE__, .text = #p, .passed = p, .test_type = CHECK};      \
    _cnn_register_test_with_case(t, __FILE__, __func__);                       \
  } while (0);

#define REQUIRE(p)                                                             \
  do {                                                                         \
    struct CNN__Test t = {                                                     \
        .lineno = __LINE__, .text = #p, .passed = p, .test_type = REQUIRE};    \
    _cnn_register_test_with_case(t, __FILE__, __func__);                       \
                                                                               \
    if (!(p)) {                                                                \
      return;                                                                  \
    }                                                                          \
  } while (0);

#define SKIP(msg)                                                              \
  do {                                                                         \
    CNN__TestCase *__cnn_current_test_case =                                   \
        _cnn_get_current_case(__FILE__, __func__);                             \
                                                                               \
    __cnn_current_test_case->status = SKIPPED;                                 \
    __cnn_current_test_case->skip_message = msg;                               \
                                                                               \
    return;                                                                    \
  } while (0);

// =================== Guarded impls ==================== //

#ifdef CATCH99_MAIN
#include <string.h>
#include <time.h>

CNN__Context *_cnn_get_context() {
  static CNN__Context ctx = {};

  return &ctx;
}

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

static CNN__TestCase *_cnn_get_current_case(const char *restrict file_name,
                                            const char *restrict fn_name) {
  CNN__Context *ctx = _cnn_get_context();
  for (int i = 0; i < ctx->num_cases; i++) {
    CNN__TestCase *test_case = &ctx->test_cases[i];

    if ((strcmp(test_case->case_fn_name, fn_name) == 0) &&
        (strcmp(test_case->filename, file_name) == 0)) {

      return test_case;
    }
  }

  return NULL;
}

void _cnn_register_case(CNN__TestCase test_case) {
  CNN__Context *ctx = _cnn_get_context();

  assert(ctx->num_cases < CATCH99_MAX_TEST_CASES &&
         "Maximum test cases exceeded");

  *(ctx->test_cases + ctx->num_cases++) = test_case;
}

void _cnn_register_test_with_case(CNN__Test t, const char *restrict file_name,
                                  const char *restrict fn_name) {
  CNN__TestCase *test_case = _cnn_get_current_case(file_name, fn_name);

  if (test_case) {
    if (test_case->num_tests >= CATCH99_MAX_TESTS) {
      fprintf(stderr, "MAXIMUM TESTS EXCEEDED FOR CASE %s\n",
              test_case->description);
      exit(1);
    }
    test_case->tests[test_case->num_tests++] = t;
  }
}

static int _cnn_get_term_width() {
#ifdef CATCH99_TERM_WIDTH
  return CATCH99_TERM_WIDTH;
#else
  struct winsize w;

  // Use ioctl to get terminal size
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    return 40;
  }

  return w.ws_col;
#endif
}

static void _cnn_print_rule(char c, int width) {
  char *arr = malloc(width + 1);
  memset(arr, c, width);
  arr[width] = '\0';

  printf("%s%s%s\n", CNN__TERM_GRAY, arr, CNN__TERM_NC);

  free(arr);
}

size_t strlen_no_ansi(const char *str) {
  size_t length = 0;
  char in_escape = 0;

  for (const char *p = str; *p != '\0'; p++) {
    if (*p == '\033' || *p == '\e') {
      in_escape = 1;
    }

    if (in_escape) {
      if (*p == 'm') {
        in_escape = 0;
      }
    } else {
      length++;
    }
  }

  return length;
}

static void _cnn_print_spaced_text(const char *lhs, const char *rhs, char sep,
                                   int width) {
  int w_l = strlen_no_ansi(lhs);
  int w_r = strlen_no_ansi(rhs);
  int padding = width - w_l - w_r;

  char *padding_str = malloc(padding + 1);
  padding_str[padding] = '\0';
  memset(padding_str, sep, padding);

  printf("%s%s%s\n", lhs, padding_str, rhs);

  free(padding_str);
}

int run_tests() {
  clock_t t0 = clock();

  CNN__Context *ctx = _cnn_get_context();

  const int width = _cnn_get_term_width();

  _cnn_print_rule('=', width);

  char now_str[64];
  _cnn_format_datetime_str(now_str);
  sprintf(now_str, "%s%s%s", CNN__TERM_GRAY, now_str, CNN__TERM_NC);

  char title[128];
  sprintf(title, "Catch99: Running %s%u%s test cases", CNN__TERM_UNDERLINE,
          ctx->num_cases, CNN__TERM_NC);

  _cnn_print_spaced_text(title, now_str, ' ', width);

  _cnn_print_rule('=', width);

  char tests_failed = 0;
  char cases_failed = 0;

  // Iterate over cases
  for (int i = 0; i < ctx->num_cases; i++) {
    CNN__TestCase *current_case = &ctx->test_cases[i];

    current_case->case_fn();

    char case_skipped = current_case->status == SKIPPED;

    // Different message formatting if skipped
    if (case_skipped) {
      char skip_summary_text[1024];
      sprintf(skip_summary_text, "%s[%3d/%3u] %s%s%s %s%s", CNN__TERM_GRAY,
              i + 1, ctx->num_cases, CNN__TERM_STRIKE,
              current_case->description, CNN__TERM_NC,
              current_case->skip_message, CNN__TERM_GRAY);

      char skip_text[64];
      sprintf(skip_text, "SKIPPED%s", CNN__TERM_NC);

      _cnn_print_spaced_text(skip_summary_text, skip_text, '.', width);

      continue;
    }

    char case_summary_txt[1024];
    sprintf(case_summary_txt, "%s[%3d/%3u]%s %s%s", CNN__TERM_GRAY, i + 1,
            ctx->num_cases, CNN__TERM_NC, current_case->description,
            CNN__TERM_GRAY);

    char results_txt[1024];
    char *p = results_txt;

    for (int t = 0; t < current_case->num_tests; t++) {
      CNN__Test test = current_case->tests[t];

      if (test.passed) {
        p += sprintf(p, "%s*%s", CNN__TERM_GREEN, CNN__TERM_NC);
      } else {
        ++tests_failed;

        // If this is the first test failed in this case, mark it down
        if (!(current_case->status == FAILED)) {
          ++cases_failed;
          current_case->status = FAILED;
        }

        p += sprintf(p, "%sF%s", CNN__TERM_RED, CNN__TERM_NC);
      }
    }

    _cnn_print_spaced_text(case_summary_txt, results_txt, '.', width);
  }

  _cnn_print_rule('-', width);

  double dt = ((double)clock() - t0) / CLOCKS_PER_SEC;
  char elapsed_str[20];
  _cnn_format_elapsed_str(dt, elapsed_str);

  if (!tests_failed) {

    char lhs[128];
    sprintf(lhs, "All tests %spassed%s", CNN__TERM_GREEN, CNN__TERM_GRAY);

    char rhs[128];
    sprintf(rhs, "%s(%s elapsed)%s", CNN__TERM_GRAY, elapsed_str, CNN__TERM_NC);

    printf("\n");
    _cnn_print_spaced_text(lhs, rhs, ' ', width);
    printf("\n");

    return 0;
  } else {

    for (int i = 0; i < ctx->num_cases; i++) {
      CNN__TestCase *current_case = &ctx->test_cases[i];

      if (!(current_case->status == FAILED)) {
        continue;
      }

      printf("\n%s%s%s\n", CNN__TERM_BOLD, current_case->description,
             CNN__TERM_NC);
      printf("%s%s:%u%s\n\n", CNN__TERM_GRAY, current_case->filename,
             current_case->lineno, CNN__TERM_NC);

      for (int t = 0; t < current_case->num_tests; t++) {
        CNN__Test test = current_case->tests[t];

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
          printf("%sFAILED:%s  %s(%s)\n", CNN__TERM_RED, CNN__TERM_NC,
                 test_type_str, test.text);
        }
      }

      printf("\n");
      _cnn_print_rule('-', width);
    }

    char lhs[128];
    sprintf(lhs, "%d tests in %d cases %sfailed%s", tests_failed, cases_failed,
            CNN__TERM_RED, CNN__TERM_GRAY);

    char rhs[128];
    sprintf(rhs, "%s(%s elapsed)%s", CNN__TERM_GRAY, elapsed_str, CNN__TERM_NC);

    printf("\n");
    _cnn_print_spaced_text(lhs, rhs, ' ', width);
    printf("\n");

    return -1;
  }
}

int main(void) { return run_tests(); }
#endif // CNN__TEST_MAIN

#endif // CATCH99
