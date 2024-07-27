#include "../eztester.h"
#include <stdio.h>
#include <unistd.h>

#define EZTESTER_IMPLEMENTATION
#include "../build/header/eztester.h"
#undef EZTESTER_IMPLEMENTATION

eztester_status shell_exists() {
  int status = eztester_shell(NULL);
  if (status) {
    return TEST_FAIL;
  } else {
    return TEST_PASS;
  }
}

eztester_status python3_exists() {
  int status = eztester_shell("/usr/bin/env python3 --version");
  if (status) {
    return TEST_FAIL;
  } else {
    return TEST_PASS;
  }
}

eztester_status sleepy() {
  eztester_log("Im feeling sleepy");
  sleep(2);
  eztester_log("zzzzzzz");
  sleep(1);
  eztester_log("ZZZZZZZZ");
  sleep(2);
  eztester_log("oh, hello there.");
  return TEST_PASS;
}

int main(int argc, char *argv[]) {
  eztester_list *list = eztester_create_list(5);

  eztester_register(
      list, (eztester_test){eztester_always_pass_test, "Always pass", 0});
  eztester_register(
      list, (eztester_test){eztester_always_warn_test, "Always warn", 0});
  eztester_register(list, (eztester_test){sleepy, "Timeout test", 4e3});
  eztester_register(list, (eztester_test){python3_exists, "Python3 Exists", 0});

  eztester_run(list, EXIT_ON_FAIL);

  return 0;
}
