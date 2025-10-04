#ifndef _EZTESTER_H
#define _EZTESTER_H

#include <stddef.h>
#include <stdio.h>

// possible results of a test.
// error is always fatal
typedef enum {
  TEST_PASS,
  TEST_WARNING,
  TEST_TIMEOUT,
  TEST_FAIL,
  TEST_ERROR
} ez_status;

/* how eztester should behave when encountering a non passing test.
 */
typedef enum {
  EXIT_NEVER = 0,
  EXIT_ON_WARNING = 1,
  EXIT_ON_TIMEOUT = 2,
  EXIT_ON_FAIL = 4
} ez_behavior;

// a single individual test to be ran
typedef ez_status(ez_runner)();

typedef struct {
  ez_runner *runner;
  const char *name;
  unsigned int max_time_ms;
} ez_test;

typedef struct {
  ez_test *tests;
  size_t length;
  size_t capacity;
} ez_list;

// create a list with a given capacity
ez_list *ez_create_list(const size_t capacity);
/* add a test to a list.
 * the same test can be registered multiple times.
 * will resize the list as necessary
 */
void ez_register(ez_list *test_list, const ez_test new_test);
// remove all tests from a list but do not free the list itself
void ez_clear_list(ez_list *test_list);
// clears a list and free's the list
void ez_destroy_list(ez_list *test_list);

// log info during a test
void ez_log(const char *__restrict format, ...);
// run all tests with a list with a given behavior
void ez_run(ez_list *test_list, const ez_behavior behavior);

/* Wrapper for `system` function
 *
 * if command is null return the availability of a shell (negated result of
 * system's behavior)
 *
 * If command is not null return the exit status of the process
 */
int ez_shell(const char *command);

// tests that always return an ez_status

// always return pass
ez_status ez_always_pass_test();
// always return warning
ez_status ez_always_warn_test();
// always return timeout
ez_status ez_always_timeout_test();
// always return fail
ez_status ez_always_fail_test();
// always return error
ez_status ez_always_error_test();

#endif // _EZTESTER_H
