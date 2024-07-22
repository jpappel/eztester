#pragma once

#include <stddef.h>
#include <stdio.h>

// possible results of a test.
// error is always fatal
typedef enum { TEST_PASS, TEST_WARNING, TEST_FAIL, TEST_ERROR } eztester_status;

/* how eztester should behave when encountering a non passing test.
 *
 * EXIT_ON_WARNING implies EXIT_ON_FAIL
 */
typedef enum { EXIT_ON_WARNING, CONTINUE_ALL, EXIT_ON_FAIL } eztester_behavior;

// a single individual test to be ran
typedef eztester_status(eztester_runner)();

typedef struct {
  eztester_runner *runner;
  const char *name;
} eztester_test;

typedef struct {
  eztester_test *tests;
  size_t length;
  size_t capacity;
} eztester_list;

// create a list with a given capacity
eztester_list *eztester_create_list(const size_t capacity);
/* add a test to a list.
 * the same test can be registered multiple times.
 * will resize the list as necessary
 */
void eztester_register(eztester_list *test_list, const eztester_test new_test);
// remove all tests from a list but do not free the list itself
void eztester_clear_list(eztester_list *test_list);
// clears a list and free's the list
void eztester_destroy_list(eztester_list *test_list);

// print during a test
void eztester_test_print(const char *__restrict format, ...);
// run all tests with a list with a given behavior
void eztester_run(eztester_list *test_list, const eztester_behavior behavior);

/* Run a shell script found at file_path, it can read in from input and write to
 * output
 *
 * Returns exit code of the command
 * UNIMPLEMENTED!
 */
unsigned char ezteser_run_shell_script(const char *file_path, FILE *input,
                                       FILE *output);

// tests that always return an eztester_status

// always return pass
eztester_status eztester_always_pass_test();
// always return warning
eztester_status eztester_always_warn_test();
// always return fail
eztester_status eztester_always_fail_test();
// always return error
eztester_status eztester_always_error_test();
