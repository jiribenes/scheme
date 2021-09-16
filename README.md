# Scheme

This Scheme interpreter _(proper name pending)_ is a tiny, readable implementation in ~2k LoC of pedantic C99.

[![Build Status](https://api.travis-ci.org/jiribenes/scheme.svg?branch=master)](https://travis-ci.org/jiribenes/scheme)

*Features:*

* Hopefully readable C99 code
* Single-pass tree-walk interpreter
* Easy embedding
* Mostly R7RS compatible
* Optional NaN tagging
* Garbage collector (simple mark and sweep)
* CL-like macro system (define-macro)
* Vector type
* Basic library
* Hash-tables made directly in Scheme
* Extensive tests

*Planned features:*

* Compiling to bytecode and running a VM
* Tail call optimization
* Character type
* UTF8 strings
* Module system

## Building

You can build the project by running `make` in the root directory.

There is a `Makefile` you can customize and `src/config.h` for 
disabling NaN tagging and/or garbage collection.

This project is buildable with Clang, GCC, TCC and strict C99/C++89 or newer on POSIX systems.

## Running

### REPL

After building the project, start the REPL by calling the binary `./scheme.out` without any additional arguments.

There you can write simple (one-line) programs and get them evaluated immediately.

### Script

After building the project, call the binary with exactly one argument 
stating the path of a program *relatively to the binary*.

A script *has to* start with a *begin* form.

Example:
```
./scheme.out examples/factorial.scm
```

## Documentation

See `doc` folder in root.
