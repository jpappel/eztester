# EZ-Tester

A barebones testing library for C on POSIX compliant systems

## Features

* lightweight
* catch SEGFAULTS and other ungraceful program exits
* no external dependencies ☺️

## Build

Note this has only been tested on my machine

* Void Linux x86_64 glibc

By default `make` builds all targets in `build/(static|dynamic|header)`

## Usage

EZ-Tester can be used as static, dynamic or header only library.
After configuring your project, create a program to run your tests.
Here is an example tester program below:

```c
#include "eztester.h"

int sum_of_integers(const int max){
    return (max*(max+1))/2;
}

eztester_status sample_test(){
    const int max = 100;
    eztester_log("Inside of Sample Test");
    eztester_log("adding %d consectuive positive integers", max);

    int actual = 0;
    for(int i = 1; i <= max; i++){
        actual += i;
    }

    int expected = sum_of_integers(max);

    if (actual == expected) {
        return TEST_PASS;
    }
    else if (actual < 0) {
        return TEST_ERROR;
    }
    else {
        return TEST_FAIL;
    }
}

int main(int argc, char* argv[]){
    eztester_list *test_list = ezterster_create_list(2);

    // runners that always return the same status are provided
    eztester_register(test_list, (eztester_test){eztester_always_pass, "Always Pass"});
    eztester_register(test_list, (eztester_test){sample_test, "Sample Test"}); // our test, can be defined in a different translation unit

    // a list will resize on register when it doesn't have capacity
    eztester_register(test_list, (eztester_test){eztester_always_fail, "Always Fail"});
    eztester_register(test_list, (eztester_test){eztester_always_warn, "Always Warn"});

    eztester_run(test_list, CONTINUE_ALL);

    eztester_destroy_list(test_list);
    return 0;
}
```

### Static

After building, copy the static libraries into your project

```bash
cp libeztester.a libeztester_debug.a $YOUR_PROJECT_DIRECORY/libs
```

When building make sure to add `-Llibs -leztester` to your linker flags

For example:
```bash
gcc -o bird_tester tests/bird_tests.c src/bird.c -Llibs -leztester
```


### Dynamic

### Header

## TODO

* [ ] makefile
    * [x] static library target
    * [x] dynamic library target
    * [ ] header-only target
* [ ] colorized output
* [ ] `run_shell_script`
