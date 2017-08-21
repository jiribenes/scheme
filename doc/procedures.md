#Procedures

Here are some basic procedures recognized by the compiler.

They split into two groups - builtins (from C, src/stdlib.scm) and stdlib (in src/stdlib.scm).

Most basic procedures have test written for them in `test/` folder.

## Conventions

* `builtin` is the prefix for unsafe C builtin procedures
* `?` is the suffix for predicates
* `!` is the suffix for mutating procedures

## Builtins

### Numeric builtins

* `builtin{+,*,-,/}` takes exactly two numbers and adds/multiplies/subtracts/divides them
* `remainder`
* `builtin{>,<,=}` takes exactly two numbers and compares them by value - returning `#t` or `#f`

### Type and predicate builtins

* `eq?` compares its arguments by value and representation in memory
    * Do not compare numbers using `eq?`, use `=`!

```scheme
(eq? 'a 'a)         ; -> #t
(eq? 0 -0)          ; -> #f
(eq? '(a b) '(a b)) ; -> #f
```

* `equal?` compares its arguments by reference

```scheme
(eq? 'a 'a)         ; -> #t
(eq? 0 -0)          ; -> #f
(eq? '(a b) '(a b)) ; -> #f
```

* `{cons,integer,number,string,symbol,procedure,vector,environment,void,undefined}?` takes one argument and returns `#t` if it's the correct type, else `#f`

### Special forms - flow control

* `begin` evaluates all its arguments and returns the return value of the last expression
* `define` bounds a symbol to a new value
    * `(define <sym> <val>)`
    * `(define (<sym> <args...>) <body>)` is used for defining a procedure
    * `(define (<sym> <arg1> <arg2> . <rest>) <body>` is for a variadic procedure
* `lambda` creates an anonymous procedure
    * `(lambda (<args...>) <body>)` makes a lambda - an unnamed procedure with arguments `<args...>` and body `<body>`
    * `(lambda <args> <body>)` makes a variadic lambda
    * `(lambda (<arg1> <arg2> . <rest>)` makes a variadic lambda with named arguments `<arg1>` and `<arg2>`
* `if`
    * `(if <cond> <then>)` returns `<then>` if `<cond>` is `#t`, else returns `#f`
    * `(if <cond> <then> <else>)` returns `<then>` if `<cond>` is `#t` else returns `<else>`
    * `(if <cond> <then> <else...>)` is equivalent to `(if <cond> <then> (begin <else...>))`
* `set!` assigns a new value for an already bound symbol
    * `(set! <sym> <val>)`
* `let` creates a new environment with certain symbol-value pairs

```scheme
(define a 20)
(displayln a)       ; -> 20
(let ((a 10))
    (displayln a))  ; -> 10
(displayln a)       ; -> 20
```

### I/O procedures

* `{write,display}` take exactly one expression and print it to stdout
    * `write` prints a string with double-quotes, `display` without
    * `write` prints a void as `#<void>`, `display` ignores it

* `newline` prints a newline to stdout
* `read` reads an S-expression from stdin
* `load` loads another scheme file and interprets it
    * Warning - the procedure takes a string of the path, which must be stated relative to the interpreter's location!

### or / and procedures

Boolean short-circuiting procedures

* `or` takes any number of arguments and sequentially evaluates them; if it encounters `#f`, returns `#f`, else evaluates all arguments and returns `#t`
* `and` takes any number of arguments and sequentially evaluates them; if it encounters `#t`, returns `#t`, else evaluates all arguments and returns `#f`

For more info, see `src/core.c` source file.

### Macro / evaluation procedures

* `define-macro` defines a new macro
    * `define-macro` acts similarly to `define`, when it is used to create a procedure
* `eval` takes an expression and an environment and evaluates the given expression in the environment, returning its result
* `apply` takes a procedure and a list and calls the procedure with the list deconstructed
* `expand` takes any expression and expands all the macros
* `quote` takes any expression and quotes it

### List procedures

See `doc/language.md`, section *Cons cells* for more information

* `cons` takes two arguments and returns a pair made from these arguments
* `c{a,d}r` are the getters for the first/second element in the pair, respectively
* `builtin-length` takes anything made from cons cells and returns an integer k
    * if the integer is zero, the list is empty
    * if the integer is positive, the list is a proper list of that length
    * if the integer is -1, the list is cyclic
    * if the integer is less than negative, the list is a dotted list of length -integer + 1

### Vector procedures

* `vector-length` takes a vector and returns its length
* `vector-ref` takes a vector and an index and returns the item at that index
* `vector-set!` takes a vector, an index and an element and sets the vector at the index to the element
* `make-vector` takes a length and an initial element and makes a vector of that length filled with the initial element

### Other library procedures

* `error` takes a string and creates a runtime error
* `current-time` returns a time in miliseconds
    * The time is bound to C's `clock()`
* `exit` very abruply quits the program
* `hash` takes any immutable element and returns it's hash

### Debug procedures

* `{current,top-level}-environment` returns an environment
* `environment-variables` returns an associative list of variables in that environment
* `environment-parent` returns the parent of an environment or an empty list

* `gc` triggers a garbage collection

## Standard library procedures

### List manipulation procedures

* `cXXr` and `cXXXr`, where `X \in {a,d}`
    * for example: `cadr` <=> `(car (cdr _))`
* `list` takes any arguments and builds a list out of them
* `null` is an alias for an empty list
* `null?` is a predicate that matches only the empty list
* `pair` <=> `cons?`

* `list?` returns `#t` if the argument is a proper list
* `length` returns the length of the list
* `list-copy` returns a (shallow) copy

* `map` maps a procedure onto a list
* `filter` takes a predicate and a list and returns a new list with elements from the former list matching the predicate
* `fold{l,r}` take a procedure, an accumulator and a list and applies the procedure recursively (to the right/to the left) on the list saving the results to the accumulator
* `reduce` is `foldl` with accumulator being the first element

* `pairs` takes a list and splits it into pairs
    * `(pairs '(1 2 3)) ; -> '((1 2) (2 3))`

* `drop` takes an integer and a list and drops the first few arguments from the list given by the integer
* `list-ref` takes an integer and a list and returns the i-th element of the list

* `member?` takes a value and a list and checks if the value is present in the list

### Numeric procedures

* `{+,*}` take any number of arguments and add/multiply them
* `{-,/}` take at least one argument and return first argument minus/divided by the sum/product of the rest

* `{=,<,>,>=,<=}` take any number of arguments and pairwise compare them

* `nan?` checks if the number is, well, not a number
* `infinite?` checks if a number is positive or negative infinity

### Vector procedures

* `vector` takes any number of arguments and returns them as a vector
* `list->vector` takes a list and returns a vector
* `vector->list` takes a vector and returns a list
* `vector-map` is like normal `map`, but for vectors
* `vector-for-each` takes a procedure and a vector and applies the procedure to every element of the vector
* `vector-fill!` takes a vector and fills it with a given element
* `vector-empty?` is a predicate for empty vectors
* `vector-append` takes two vectors and makes a new one out of the two combined

### Hash-table procedures

* `make-hash` takes an equality predicate and returns a new, empty hash-table
* `hash-{eq,count,capacity,data}` returns the equality predicate/count/capacity/data vector of a hash
* `hash?` is the hash predicate
* `hash-entry` is an internal procedure for getting if a key is in a hash-table or if not, where to put it
* `hash-exists?` returns if an element is present in the hash-table
* `hash-{ref,set!,delete}` looks-up/sets/deletes an element in the hash-table
* `hash-maybe-resize!` is an internal procedure for checking if a resize should be made
* `hash-resize!` resizes a hash-table to a given capacity
* `hash-walk` takes a hash-table and a procedure with two arguments and uses it on every (key, value) pair in the table
* `hash->alist` converts the table into an association list

### Standard macros

* `{when,unless}`

### Other procedures

* `test`, `not`, `{write,display}ln`
