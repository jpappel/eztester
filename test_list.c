#include "eztester.h"

#include <assert.h>
#include <stdlib.h>

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
  list->tests = tests;

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

void eztester_clear_list(eztester_list *test_list) {
  free(test_list->tests);
  test_list->length = 0;
  test_list->capacity = 0;
}

void eztester_destroy_list(eztester_list *test_list) {
  eztester_clear_list(test_list);
  free(test_list);
}
