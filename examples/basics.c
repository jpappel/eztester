#include "../eztester.h"
#include <stdio.h>
#include <unistd.h>

#define EZTESTER_IMPLEMENTATION
#include "../build/header/eztester.h"
#undef EZTESTER_IMPLEMENTATION

ez_status shell_exists() {
  int status = ez_shell(NULL);
  if (status) {
    return TEST_FAIL;
  } else {
    return TEST_PASS;
  }
}

ez_status python3_exists() {
  int status = ez_shell("/usr/bin/env python3 --version");
  if (status) {
    return TEST_FAIL;
  } else {
    return TEST_PASS;
  }
}

ez_status sleepy() {
  ez_log("Im feeling sleepy");
  sleep(2);
  ez_log("zzzzzzz");
  sleep(1);
  ez_log("ZZZZZZZZ");
  sleep(2);
  ez_log("oh, hello there.");
  return TEST_PASS;
}

int main(int argc, char *argv[]) {
  ez_list *list = ez_create_list(5);

  ez_register(
      list, (ez_test){ez_always_pass_test, "Always pass", 0});
  ez_register(
      list, (ez_test){ez_always_warn_test, "Always warn", 0});
  ez_register(list, (ez_test){sleepy, "Timeout test", 4e3});
  ez_register(list, (ez_test){python3_exists, "Python3 Exists", 0});

  ez_run(list, EXIT_ON_FAIL);

  return 0;
}
