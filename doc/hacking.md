# Hacking

This file contains information on how to change something in the project.

## Project structure

```
|-- doc
|   |-- embedding.md    <-- how to embed this interpreter in your program
|   |-- hacking.md      <-- a guide to interpreter's internals (this very file!)
|   |-- language.md     <-- a language guide (mostly info about different value types)
|   `-- procedure.md    <-- a list of all procedures provided by the interpreter
|-- examples            <== various examples of implemented scheme language
|   `-- ...
|-- include
|   `-- scheme.h        <-- C header file containing everything you should need to embed this interpreter in your programs
|-- src
|   |-- config.h        <-- a basic config for enabling/disabling features
|   |-- core.{c,h}      <-- contains the core procedures and forms
|   |-- read.{c,h}      <-- C functions for reading - parsing, lexing
|   |-- scheme.c        <-- a tiny wrapper around the interpreter library, the front-end
|   |-- stdlib.scm      <-- a standard library written in scheme, loaded by the interpreter
|   |-- value.{c,h}     <-- describes the data/value types used by the interpreter
|   |-- vm.{c,h}        <-- contains the interpreter and its methods
|   `-- write.{c,h}     <-- C functions for writing - printing, displaying
`-- test                <== various tests for the different parts of the interpreter
```

## Formatting

This project uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) with custom settings.
To format the file, install clang-format and do `make format` in root directory.
