#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../../runtime.h"
#include "./wasi_spec.h"

typedef struct wasi_options_s {
    __wasi_size_t argc;
    const char**  argv;
    const char**  envp;
} wasi_options_t;

static inline void wasi_options_init(wasi_options_t* options) {
    awsm_assert(options != NULL);

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

static inline __wasi_errno_t wasi_unsupported_syscall(const char* syscall) {
#ifndef NDEBUG
    fprintf(stderr, "Syscall %s is not supported\n", syscall);
#endif
    return __WASI_ERRNO_NOTSUP;
}
