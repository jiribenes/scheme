#ifndef _core_h
#define _core_h

#include <stdarg.h>

#include "config.h"
#include "scheme.h"
#include "value.h"

// Called when a runtime error is encountered
void error_runtime(vm_t *vm, const char *format, ...);

// Checks if there are exactly n arguments (if at_least is false)
//                  or at least n arguments (if at_least is true)
bool arity_check(vm_t *vm, const char *fn_name, value_t args, int n,
                 bool at_least);

// Loads a default environment
env_t *scm_env_default(vm_t *vm);

#endif  // _core_h
