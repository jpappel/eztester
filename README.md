# EZ-Tester

A barebones testing library for C

## Build

Note this has only been tested on my machine

* Void Linux x86_64 glibc

```bash
make static # builds libeztester.a and libeztester_debug.a
```

### Dependencies

* None ☺️

## Usage

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
    * [ ] dynamic library target
    * [ ] header-only target
* [ ] colorized output
* [ ] `run_shell_script`
