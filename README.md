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
Make sure to copy `eztester.h` somewhere in your include path.
After configuring your project, create a program to run your tests.

<details>
<summary>Example Program</summary>

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

eztester_status sample_shell_test(){
    // eztester_shell is a wrapper function for `system`
    // see `eztester.h` for more info
    int status = eztester_shell("curl invalid.url");
    if (status == 0){
        return TEST_PASS;
    }
    else if (status == 6){
        return TEST_WARNING;
    }
    else if (status == 127){
        return TEST_ERROR;
    }
    else {
        return TEST_FAIL;
    }
}

int main(int argc, char* argv[]){
    eztester_list *tests = ezterster_create_list(2);

    // runners that always return the same status are provided
    eztester_register(tests, (eztester_test){eztester_always_pass, "Always Pass"});
    eztester_register(tests, (eztester_test){sample_test, "Sample Test"}); // our test, can be defined in a different translation unit

    // a list will resize on register when it doesn't have capacity
    eztester_register(tests, (eztester_test){eztester_always_fail, "Always Fail"});
    eztester_register(tests, (eztester_test){eztester_always_warn, "Always Warn"});
    
    eztester_register(tests, (eztester_test){sample_shell_test, "Check a non existent url");

    eztester_run(tests, EXIT_ON_FAIL | EXIT_ON_TIMEOUT );

    eztester_destroy_list(tests);
    return 0;
}
```

</details>

### Static

After building, copy the static libraries into your project

```bash
cp build/static/* $YOUR_PROJECT_DIRECORY/libs
```

When building make sure to add `-Llibs -leztester` to your linker flags

For example:
```bash
gcc -o tests test/test.c src/module.c -Llibs -leztester
```

### Dynamic

### Header

To use the header-only implementation replace the include in your programs entry point to:

```c
#define EZTESTER_IMPLEMENTATION
#include "eztester.h"
#undef EZTESTER_IMPLEMENTATION
```

## TODO

* [x] makefile
    * [x] static library target
    * [x] dynamic library target
    * [x] header-only target
* [x] shell command utility
* [ ] colorized output
