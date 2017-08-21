# Language

Basic language has two types of values - basic values (constants) 
and pointer values (ptrvalues).

## Values

All basic values are self-evaluating.

### Number

Number is the only basic numeric type available in Scheme.
It corresponds to a 64bit float - type 'double' in C.

Predicates - `number?`, `integer?` (checks if number is integer)

Forms - 1, 0, -10, 420, 10.5, -1e9, 1.5e-20, +inf.0, +nan.0

### Boolean

Represents pure true and false values - #t, #f respectively

Predicate - `boolean?`

### Void, undefined, EOF

Void is a special value representing the absence of value.
It is mainly used as an expression for no return value.
Procedure `void` takes any number of arguments and returns `#<void>`

Undefined is a constant - `#<undefined>`
It is used as a result of referencing an unavailable value.

EOF is a special constant returned when getting the end of file - `#<eof>`

### Null

Null, written as '(), represents an empty list

## Ptrvalues

### Cons list

The `cons` procedure takes two arguments and puts them in a pair.
You can extract the first and second elements of the pair 
with `car` and `cdr` respectively.

```
>> (cons 1 2)
(1 . 2)

>> (car (cons 1 2))
1
```

A proper list is a linked list created with pairs.
It's either the empty list - `null` or a pair where 
the first element is an element of the list and the second is a list.
You can use the predicate `list?` to recognize proper lists.

A dotted list is a proper list with last element being a pair.

### String

A string is a fixed-length array of characters.
Strings are self-evaluating.

```
>> "Hello"
"Hello"
```

### Symbol

A symbol is a case-sensitive identifier.
All symbols with the same sequence of characters are interned,
meaning there is only one symbol created for the same identifier.

```
>> 'sym
'sym

>> (symbol? 'sym)
#t
```

The 'gensym' procedure creates an uninterned symbol
that is not equal to any other symbol.

### Vector

A vector is a (dynamic) array of values.
It can be created with `vector`, `make-vector` and `list->vector` procedures.

Predicate - `vector?`

### Hash-table

Hash-tables are implemented directly in Scheme.
They are growable hash-tables with linear probing.

## Differences to S7RS

[S7RS-small](http://trac.sacrideo.us/wg/wiki/R7RSHomePage)

* No bytevectors
* No characters
* No proper ports
* No exceptions
* No call/cc
* No tail recursion
