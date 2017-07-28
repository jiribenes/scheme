#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

static vm_t *vm_init() {
    scm_config_t config;
    scm_config_default(&config);

    config.error_fn = error_report;

    // 64 MB
    config.heap_size_initial = 1024 * 1024 * 64;

    vm_t *vm = vm_new(&config);

    return vm;
}

// Returns null if file not read
static char *file_read(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) return NULL;

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);

    char *buffer = (char *) malloc(fsize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Could not read file %s (buffer alloc fail)!\n",
                filename);
        exit(74);  // EX_IOERR
    }

    size_t read = fread(buffer, sizeof(char), fsize, f);
    if (read < fsize) {
        fprintf(stderr, "ERROR: Could not read file %s (whole not read)!\n",
                filename);
        exit(74);  // EX_IOERR
    }

    buffer[read] = '\0';

    fclose(f);
    return buffer;
}

void library_read(vm_t *vm, env_t *env, const char *library) {
#if DEBUG
    fprintf(stdout, "DEBUG: Reading library %s!\n", library);
#endif  // DEBUG
    char *source = file_read(library);
    if (source == NULL) {
        fprintf(stderr, "ERROR: Could not find library %s!\n", library);
        exit(66);  // EX_NOINPUT
    }
    value_t val = read_source(vm, source);
    eval(vm, env, val);
}

/* *** */

void file_run(const char *filename) {
    char *source = file_read(filename);
    if (source == NULL) {
        fprintf(stderr, "ERROR: Could not find file %s!\n", filename);
        exit(66);  // EX_NOINPUT
    }

    vm_t *vm = vm_init();
    env_t *env = scm_env_default(vm);
    library_read(vm, env, "src/stdlib.scm");

    value_t val = read_source(vm, source);
    value_t result = eval(vm, env, val);

    fprintf(stdout, "Result:");
    display(stdout, result);
    fprintf(stdout, "\n");

    vm_free(vm);
    free(source);
}

void repl_run() {
    vm_t *vm = vm_init();
    env_t *env = scm_env_default(vm);
    library_read(vm, env, "src/stdlib.scm");

    fprintf(stdout, " _  __     \n"
                    "(_ /  |\\/|\n"
                    "__)\\__|  |\n"
                    "  v%s  \n\n"
                    "Welcome to the REPL!\n"
                    "Ctrl+D to exit!\n\n",
            SCM_VERSION_STRING);

    char line[1024];

    while (true) {
        fprintf(stdout, ">> ");

        if (!fgets(line, 1024, stdin)) {
            fprintf(stdout, "\n");
            break;
        }

        value_t val = read_source(vm, line);
        value_t result = eval(vm, env, val);

        if (!IS_VOID(result)) {
            display(stdout, result);
            fprintf(stdout, "\n");
        }
    }

    fprintf(stdout, "Quitting!\n");
    vm_free(vm);
}

/* *** */
int main(int argc, char *argv[]) {
    // TODO: Add support for cmdline arguments for scripts
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0) {
            fprintf(stdout, "Usage: scheme.out [file]\n");
            fprintf(stdout, "  --help    : Show this help\n");
            fprintf(stdout, "  --version : Show version\n");
            return 0;
        } else if (strcmp(argv[1], "--version") == 0) {
            fprintf(stdout, "SCM v%s\n", SCM_VERSION_STRING);
            return 0;
        }
    }

    if (argc == 1) {
        repl_run();
    } else {
        file_run(argv[1]);
    }

    return 0;
}
