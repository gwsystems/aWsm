#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "./wasi_spec.h"

typedef struct wasi_options_s {
    __wasi_size_t argc;
    const char**  argv;
    const char**  envp;
} wasi_options_t;

static inline void wasi_options_init(wasi_options_t* options) {
    assert(options != NULL);

    options->argc = 0;
    options->argv = NULL;
    options->envp = NULL;
}

void*         wasi_context_init(wasi_options_t* options);
void          wasi_context_destroy(void* handle);
__wasi_size_t wasi_context_get_argc(void* wasi_context);
char**        wasi_context_get_argv(void* wasi_context);
__wasi_size_t wasi_context_get_argv_buf_size(void* wasi_context);
__wasi_size_t wasi_context_get_envc(void* wasi_context);
__wasi_size_t wasi_context_get_env_buf_size(void* wasi_context);

__attribute__((noreturn)) static inline void wasi_unsupported_syscall(const char* syscall) {
    fprintf(stderr, "Syscall %s is not supported\n", syscall);
    exit(EXIT_FAILURE);
}
