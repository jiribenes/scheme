#include <stdarg.h>
#include <stdio.h>

#include "scheme.h"

#include "core.h"
#include "read.h"
#include "value.h"
#include "vm.h"
#include "write.h"

static void error_report(vm_t *vm, int line, const char *message) {
    if (line > 0) {
        fprintf(stderr, "ERROR @ line %d: %s\n", line, message);
    } else if (line == -1) {
        fprintf(stderr, "ERROR @ runtime: %s\n", message);
    }
}

/* *** */

void repl(vm_t *vm, env_t *env) {
    fprintf(stdout,
            "|Scheme " SCM_VERSION_STRING " - REPL|\n|Use Ctrl+C to exit!|\n");
    char buf[512];

    while (true) {
        fprintf(stdout, ">> ");

        while (fgets(buf, 512, stdin) == NULL) {
            break;
        }
        value_t val = read_source(vm, buf);

        value_t result = eval(vm, env, val);

        write(stdout, result);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "Quiting!\n");
}

int main(int argc, char *argv[]) {
    scm_config_t config;
    scm_config_default(&config);

    config.error_fn = error_report;

    vm_t *vm = vm_new(&config);
    env_t *env = scm_env_default(vm);

    if (argc == 1) repl(vm, env);
    if (argc == 2) {
        value_t val = file_read(vm, argv[1]);
        value_t result = eval(vm, env, val);

        fprintf(stdout, "Result: ");
        write(stdout, result);
        fprintf(stdout, "\n");
    }

    vm_free(vm);
    return 0;
}
