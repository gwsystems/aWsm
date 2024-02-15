#include <stdlib.h>

#include "../../runtime.h"
#include "wasi_impl.h"

/* This is simple startup code that initializes a single sandbox, runs it to completion, and then exits
 * It assumes a Unix environment and has a 1:1 relationship between a Unix process and a WebAssembly sandbox.
 * This likely may not fit for your requirements. Look at README.md in this directory for guidance on
 * writing custom startup logic for other host environments and execution models.
 */

void*                current_wasi_context;
struct dylib_handler so_handler;

/* Code that actually runs the wasm code */
IMPORT void wasmf__start(void);

/**
 * @brief Lifecycle function that runs at end at termination of WebAssembly module. Suitable for logging and cleanup
 */
void runtime_on_module_exit() {
    wasi_context_destroy(current_wasi_context);
    return;
}

int main(int argc, char* argv[]) {
    so_handler.app_path = argv[0];

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

    if (so_handler.handle) {
        so_handler.entrypoint();
    } else {
        wasmf__start();
    }

    exit(EXIT_SUCCESS);
}
