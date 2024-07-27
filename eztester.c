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

struct _ez_shared_mem {
  int work_in_queue : 1;
  eztester_status status : 3;
  eztester_behavior behavior : 3;
  size_t index;
};

struct _ez_tests_results {
  size_t current;
  size_t passed;
  size_t total;
};

volatile sig_atomic_t _ez_child_premature_exit = 0;
volatile sig_atomic_t _ez_child_premature_exit_signal;
volatile sig_atomic_t _ez_child_premature_exit_status = 0;

struct _ez_shared_mem *_ez_create_shared_memory() {
  const size_t shm_size = sizeof(struct _ez_shared_mem);
  char shr_mem_name[32];
  sprintf(shr_mem_name, "/eztester-%d", getpid());

  int shm_fd = shm_open(shr_mem_name, O_RDWR | O_CREAT, 0666);
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

  return shm_ptr;
}

void _ez_destroy_shared_memory(struct _ez_shared_mem *mem) {
  char shr_mem_name[32];
  sprintf(shr_mem_name, "/eztester-%d", getpid());

  // unmap memory
  if (munmap(mem, sizeof(struct _ez_shared_mem)) == -1) {
    perror("munmap");
    exit(1);
  }
  // unlink shared memory
  if (shm_unlink(shr_mem_name) == -1) {
    perror("shm_unlink");
    exit(1);
  }
}

void _ez_print_test_results(const struct _ez_tests_results results) {
  printf("\n--------\nRan %zu of %zu tests\n", results.current, results.total);
  if (results.current == results.total && results.current == results.passed) {
    printf("All tests passed :)\n");
  } else if (results.current == results.total && results.passed == 0) {
    printf("No test passsed :(\n");
  } else {
    printf("%zu of %zu tests passed\n", results.passed, results.current);
  }
}

void _ez_premature_exit(const char *message, const pid_t worker,
                        struct _ez_shared_mem *mem,
                        const struct _ez_tests_results results) {
  fprintf(stderr, "%s\n", message);
  if (_ez_child_premature_exit) {
    if (_ez_child_premature_exit_status != 0) {
      fprintf(stderr, "Worker Process exited with status: %d\n",
              _ez_child_premature_exit_status);
    }
    if (_ez_child_premature_exit_signal > 0) {
      fprintf(stderr, "Worker Process exited because of signal: %s\n",
              strsignal(_ez_child_premature_exit_signal));
    }
  }
  kill(worker, SIGTERM);
  wait(NULL);
  _ez_print_test_results(results);

  _ez_destroy_shared_memory(mem);
  exit(1);
}

void _ez_worker(volatile struct _ez_shared_mem *mem,
                const eztester_list *list) {
  while (mem->index < list->length) {
    // wait for work
    while (!mem->work_in_queue) {
      usleep(50e3);
    }
    mem->status = list->tests[mem->index].runner();

    // check if worker should die
    if (mem->status == TEST_ERROR ||
        (mem->status == TEST_FAIL && mem->behavior & EXIT_ON_FAIL) ||
        (mem->status == TEST_WARNING && mem->behavior & EXIT_ON_WARNING) ||
        (mem->status == TEST_TIMEOUT && mem->behavior & EXIT_ON_TIMEOUT)) {
      exit(1);
    }

    mem->work_in_queue = false;
    raise(SIGSTOP);
  }
  exit(0);
}

void _ez_chld_handler(int signum) {
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    if (WIFEXITED(status)) {
      _ez_child_premature_exit_status = WEXITSTATUS(status);
      _ez_child_premature_exit = 1;
    } else if (WIFSIGNALED(status)) {
      _ez_child_premature_exit_signal = WTERMSIG(status);
      _ez_child_premature_exit = 1;
    }
  }
}

void eztester_run(eztester_list *test_list, eztester_behavior behavior) {

  struct _ez_shared_mem *mem = _ez_create_shared_memory();
  mem->index = 0;
  mem->work_in_queue = false;
  mem->behavior = behavior;

  pid_t pid, child_pgid;
  pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    setpgrp();
    _ez_worker(mem, test_list);
  }
  child_pgid = pid;

  eztester_status status;
  eztester_test test;
  struct _ez_tests_results results = {
      .current = 0, .passed = 0, .total = test_list->length};

  // set child signal handler
  signal(SIGCHLD, _ez_chld_handler);

  for (size_t i = 0; i < test_list->length; i++) {
    test = test_list->tests[i];

    results.current = i + 1;
    mem->index = i;
    mem->work_in_queue = true;

    printf("[%03zu/%03zu] Testing: %s\n", results.current, results.total,
           test.name);
    fflush(stdout);

    kill(pid, SIGCONT);

    unsigned int elapsed_ms = 0;
    while (mem->work_in_queue) {
      usleep(1e3);
      elapsed_ms++;
      if (_ez_child_premature_exit) {
        _ez_premature_exit("Worker Process ended prematurely!", pid, mem,
                           results);
      }
      if (test.max_time_ms > 0 && elapsed_ms > test.max_time_ms) {

        int status;
        signal(SIGCHLD, SIG_DFL);

        // ask nicely
        killpg(child_pgid, SIGTERM);
        usleep(10e3);
        if (waitpid(pid, &status, WNOHANG) == 0) {
          // no longer ask nicely
          killpg(child_pgid, SIGKILL);
        }

        pid = fork();
        if (pid < 0) {
          perror("fork");
          exit(1);
        } else if (pid == 0) {
          setpgrp();
          _ez_worker(mem, test_list);
        }
        child_pgid = pid;
        signal(SIGCHLD, _ez_chld_handler);

        mem->work_in_queue = false;
        mem->status = TEST_TIMEOUT;
      }
    }

    status = mem->status;

    switch (status) {
    case TEST_PASS:
      printf("[%03zu/%03zu] %s Result: Pass\n", results.current, results.total,
             test.name);
      results.passed++;
      break;

    case TEST_WARNING:
      printf("[%03zu/%03zu] %s Result: Warning\n", results.current,
             results.total, test.name);
      if (behavior & EXIT_ON_WARNING) {
        _ez_premature_exit("Warning occured, Exitting", pid, mem, results);
      }
      results.passed++;
      break;

    case TEST_TIMEOUT:
      printf("[%03zu/%03zu] %s Result: Timeout\n", results.current,
             results.total, test.name);
      if (behavior & EXIT_ON_TIMEOUT) {
        _ez_premature_exit("Timeout occured, Exitting", pid, mem, results);
      }
      break;

    case TEST_FAIL:
      printf("[%03zu/%03zu] %s Result: Fail\n", results.current, results.total,
             test.name);
      if (behavior & EXIT_ON_FAIL) {
        _ez_premature_exit("Failure occured, Exitting", pid, mem, results);
      }
      break;

    case TEST_ERROR:
      printf("[%03zu/%03zu] %s Result: Error\n", results.current, results.total,
             test.name);
      _ez_premature_exit("Fatal Error occured! Exiting", pid, mem, results);
      break;
    }
  }

  signal(SIGCHLD, SIG_DFL);
  _ez_print_test_results(results);

  _ez_destroy_shared_memory(mem);
}

int eztester_shell(const char *command) {
  int result;
  if (command == NULL) {
    eztester_log("Recieved NULL as command, checking for shell availability");
    result = !system(command);
    eztester_log("Shell %s available", (result) ? "is not" : "is");
    return result;
  }

  eztester_log("Executing %s", command);
  result = system(command);
  if (result == -1) {
    eztester_log("Error with child process");
    perror(command);
  } else {
    eztester_log("Process exited with a status of %d", result);
  }

  return result;
}

void eztester_log(const char *restrict format, ...) {
  va_list args;
  va_start(args, format);
  printf(">  ");
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

eztester_status eztester_always_pass_test() { return TEST_PASS; }
eztester_status eztester_always_warn_test() { return TEST_WARNING; }
eztester_status eztester_always_timeout_test() { return TEST_TIMEOUT; }
eztester_status eztester_always_fail_test() { return TEST_FAIL; }
eztester_status eztester_always_error_test() { return TEST_ERROR; }
