# Embedding

For example, see `src/scheme.c`.

## Configuration struct

Always use `scm_config_default` to load the defaults before changing its members.

* `realloc_fn` - a function for allocating, reallocating and freeing memory
* `error_fn` - a function for reporting an error to the user
* `load_fn` - a function for loading scripts
* initial, minimum heap size (in bytes)
* heap growth (between 0 and 1)

## How to embed

First, customize the configuration (see above).
Then call `vm_new` to create a VM object.

After that, call `scm_env_default` to create a default environment.
Note: You don't have to do this! Look at `src/core.c` where it is defined to see what it is actually doing.

If you want to read (parse, lex) an expression, use `read_source`.
Use `eval` to evaluate an expression.

Do not forget to free the VM - `vm_free`.

### Adding a builtin procedure / a variable.

Ensure you have a valid, initialized environment.

Use `primitive_add` or `variable_add` from `src/vm.h`.

See example in `scm_env_default` in `src/core.c`.
