#include <stdlib.h>

#include "../../runtime.h"
#include "wasi_impl.h"

void* current_wasi_context;


/* Code that actually runs the wasm code */
IMPORT void wasmf__start(void);

#define unlikely(x) __builtin_expect(!!(x), 0)

/**
 * @brief Lifecycle function that runs at end at termination of WebAssembly module. Suitable for logging and cleanup
 */
void runtime_on_module_exit() {
    wasi_context_destroy(current_wasi_context);
    return;
}

int main(int argc, char* argv[]) {
    runtime_init();

    /* Copy environ from process */
    extern char** environ;
    __wasi_size_t envc = 0;
    for (char** cursor = environ; *cursor != NULL; cursor++) {
        envc++;
    }

    wasi_options_t options;
    wasi_options_init(&options);
    options.argc         = argc;
    options.argv         = (const char**)argv;
    options.envp         = (const char**)environ;
    current_wasi_context = wasi_context_init(&options);

    atexit(runtime_on_module_exit);

    wasmf__start();
    exit(EXIT_SUCCESS);
}
