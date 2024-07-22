#include "eztester.h"
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <stdbool.h>

struct shared_mem {
  int work_in_queue : 1;
  eztester_status status : 2;
  eztester_behavior behavior : 2;
  size_t index;
};

volatile sig_atomic_t child_premature_exit = 0;
volatile sig_atomic_t child_premature_exit_signal;
volatile sig_atomic_t child_premature_exit_status = 0;

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

void premature_exit(const char *message, const pid_t worker,
                    const size_t current_test, const size_t tests_passed,
                    const size_t num_tests) {
  fprintf(stderr, "%s\n", message);
  if (child_premature_exit) {
    if (child_premature_exit_status != 0) {
      fprintf(stderr, "Worker Process exited with status: %d\n",
              child_premature_exit_status);
    }
    if (child_premature_exit_signal > 0) {
      fprintf(stderr, "Worker Process exited because of signal: %s",
              strsignal(child_premature_exit_signal));
    }
  }
  kill(worker, SIGTERM);
  wait(NULL);
  print_test_results(current_test, tests_passed, num_tests);
  exit(1);
}

void worker(volatile struct shared_mem *mem, const eztester_list *list) {
  while (mem->index < list->length) {
    // wait for work
    while (!mem->work_in_queue) {
      usleep(1000 * 50);
    }
    mem->status = list->tests[mem->index].runner();

    // check if worker should die
    if (mem->status == TEST_ERROR ||
        (mem->status == TEST_FAIL && mem->behavior != CONTINUE_ALL) ||
        (mem->status == TEST_WARNING && mem->behavior == EXIT_ON_FAIL)) {
      exit(1);
    }

    mem->work_in_queue = false;
  }
  exit(0);
}

void chld_handler(int signum) {
  int status;
  pid_t pid;
  child_premature_exit = 1;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (WIFEXITED(status)) {
      child_premature_exit_status = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      child_premature_exit_signal = WTERMSIG(status);
    }
  }
}

void eztester_run(eztester_list *test_list, eztester_behavior behavior) {
  const size_t shm_size = sizeof(struct shared_mem);
  int shm_fd = shm_open("/test_queue", O_RDWR | O_CREAT, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  if (ftruncate(shm_fd, shm_size) == -1) {
    perror("ftruncate");
    exit(1);
  }

  void *shm_ptr =
      mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  struct shared_mem *mem = shm_ptr;
  mem->index = 0;
  mem->work_in_queue = false;
  mem->behavior = behavior;

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    worker(mem, test_list);
  }

  eztester_status status;
  eztester_test test;
  const size_t length = test_list->length;
  size_t pass_count = 0;

  // set child signal handler
  signal(SIGCHLD, chld_handler);

  for (size_t i = 0; i < test_list->length; i++) {
    test = test_list->tests[i];

    mem->index = i;
    mem->work_in_queue = true;

    printf("[%03zu/%03zu] Testing: %s\n", i + 1, length, test.name);
    fflush(stdout);
    if (child_premature_exit) {
      premature_exit("Worker Process ended prematurely!", pid, i + 1,
                     pass_count, length);
    }
    while (mem->work_in_queue) {
      usleep(1000 * 50);
      if (child_premature_exit) {
        premature_exit("Worker Process ended prematurely!", pid, i + 1,
                       pass_count, length);
      }
    }
    status = mem->status;

    switch (status) {
    case TEST_PASS:
      printf("[%03zu/%03zu] %s Result: Pass\n", i + 1, length, test.name);
      pass_count++;
      break;

    case TEST_WARNING:
      printf("[%03zu/%03zu] %s Result: Warning\n", i + 1, length, test.name);
      if (behavior == EXIT_ON_WARNING) {
        premature_exit("Warning occured, Exitting", pid, i + 1, pass_count,
                       length);
      }
      break;

    case TEST_FAIL:
      printf("[%03zu/%03zu] %s Result: Fail\n", i + 1, length, test.name);
      if (behavior != CONTINUE_ALL) {
        premature_exit("Failure occured, Exitting", pid, i + 1, pass_count,
                       length);
      }
      break;

    case TEST_ERROR:
      printf("[%03zu/%03zu] %s Result: Error\n", i + 1, length, test.name);
      premature_exit("Fatal Error occured! Exiting", pid, i + 1, pass_count,
                     length);
      break;
    }
  }
  signal(SIGCHLD, SIG_DFL);
  print_test_results(length, pass_count, length);

  // unmap memory
  if (munmap(shm_ptr, shm_size) == -1) {
    perror("munmap");
    exit(1);
  }
  // unlink shared memory
  if (shm_unlink("/test_queue") == -1) {
    perror("shm_unlink");
    exit(1);
  }
}

void eztester_test_print(const char *restrict format, ...) {
  va_list args;
  va_start(args, format);
  printf(">  ");
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

eztester_status eztester_always_pass_test() { return TEST_PASS; }
eztester_status eztester_always_warn_test() { return TEST_WARNING; }
eztester_status eztester_always_fail_test() { return TEST_FAIL; }
eztester_status eztester_always_error_test() { return TEST_ERROR; }
