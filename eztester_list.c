#include "eztester.h"

#include <assert.h>
#include <stdlib.h>

ez_list *ez_create_list(const size_t capacity) {
  ez_list *list = malloc(sizeof(ez_list));
  if (!list) {
    return NULL;
  }

  list->length = 0;

  if (capacity == 0) {
    list->tests = NULL;
    list->capacity = 0;
    return list;
  }

  ez_test *tests = malloc(capacity * sizeof(ez_test));
  if (!tests) {
    free(list);
    return NULL;
  }
  list->tests = tests;

  list->capacity = capacity;
  return list;
}

void ez_register(ez_list *test_list, const ez_test new_test) {
  if (test_list->capacity == 0) {
    test_list->tests = realloc(test_list->tests, 2 * sizeof(ez_test));
    test_list->capacity = 2;
  }
  if (test_list->capacity <= test_list->length + 1) {
    test_list->capacity *= 2;
    test_list->tests =
        realloc(test_list->tests, test_list->capacity * sizeof(ez_test));
    assert(test_list->tests);
  }

  test_list->tests[test_list->length++] = new_test;
}

void ez_clear_list(ez_list *test_list) {
  free(test_list->tests);
  test_list->length = 0;
  test_list->capacity = 0;
}

void ez_destroy_list(ez_list *test_list) {
  ez_clear_list(test_list);
  free(test_list);
}
