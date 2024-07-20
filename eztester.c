#include "eztester.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

eztester_list *eztester_create_list(const size_t capacity) {
  eztester_list *list = malloc(sizeof(eztester_list));
  if (!list) {
    return NULL;
  }

  list->length = 0;

  if (capacity == 0) {
    list->tests = NULL;
    list->capacity = 0;
    return list;
  }

  eztester_test *tests = malloc(capacity * sizeof(eztester_test));
  if (!tests) {
    free(list);
    return NULL;
  }

  list->capacity = capacity;
  return list;
}
void eztester_register(eztester_list *test_list, const eztester_test new_test) {
  if (test_list->capacity == 0) {
    test_list->tests = realloc(test_list->tests, 2 * sizeof(eztester_test));
    test_list->capacity = 2;
  }
  if (test_list->capacity <= test_list->length + 1) {
    test_list->capacity *= 2;
    test_list->tests =
        realloc(test_list->tests, test_list->capacity * sizeof(eztester_test));
    assert(test_list->tests);
  }

  test_list->tests[test_list->length++] = new_test;
}

void print_test_results(const size_t tests_run, const size_t tests_passed,
                        const size_t num_tests) {
  printf("--------\nRan %zu of %zu tests\n", tests_run, num_tests);
  if (tests_run == num_tests && tests_run == tests_passed) {
    printf("All tests passed :)\n");
  } else if (tests_run == num_tests && tests_passed == 0) {
    printf("No test passsed :(\n");
  } else {
    printf("%zu of %zu tests passed\n", tests_passed, tests_run);
  }
}

void eztester_run(eztester_list *test_list, eztester_behavior behavior) {
  eztester_status status;
  eztester_test test;
  const size_t length = test_list->length;
  size_t pass_count = 0;

  for (size_t i = 0; i < test_list->length; i++) {
    test = test_list->tests[i];
    printf("[%03zu/%03zu] Testing: %s\n", i + 1, length, test.name);
    fflush(stdout);
    status = test.runner();

    switch (status) {
    case TEST_PASS:
      printf("[%03zu/%03zu] %s Result: Pass\n", i + 1, length, test.name);
      pass_count++;
      break;

    case TEST_WARNING:
      printf("[%03zu/%03zu] %s Result: Warning\n", i + 1, length, test.name);
      if (behavior == EXIT_ON_WARNING) {
        printf("Warning occured, Exitting\n");
        print_test_results(i + 1, pass_count, length);
        exit(1);
      }
      break;

    case TEST_FAIL:
      printf("[%03zu/%03zu] %s Result: Fail\n", i + 1, length, test.name);
      if (behavior != CONTINUE_ALL) {
        printf("Failure occured, Exitting\n");
        print_test_results(i + 1, pass_count, length);
        exit(1);
      }
      break;

    case TEST_ERROR:
      printf("[%03zu/%03zu] %s Result: Error\nFatal Error occured! Exiting\n",
             i + 1, length, test.name);
      print_test_results(i + 1, pass_count, length);
      exit(1);
      break;
    }
  }
  print_test_results(length, pass_count, length);
}

void eztester_test_print(const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf(">  ");
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

void eztester_clear_list(eztester_list *test_list) {
  free(test_list->tests);
  test_list->length = 0;
  test_list->capacity = 0;
}

void eztester_destroy_list(eztester_list *test_list) {
  eztester_clear_list(test_list);
  free(test_list);
}

eztester_status eztester_always_pass_test() { return TEST_PASS; }
eztester_status eztester_always_warn_test() { return TEST_WARNING; }
eztester_status eztester_always_fail_test() { return TEST_FAIL; }
eztester_status eztester_always_error_test() { return TEST_ERROR; }
