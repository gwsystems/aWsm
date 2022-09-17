#include "runtime.h"
#include <fenv.h>
#include <math.h>

struct indirect_table_entry indirect_table[INDIRECT_TABLE_SIZE];

// TODO: Throughout here we use `assert` for error conditions, which isn't optimal
// Instead we should use `unlikely` branches to a single trapping function (which should optimize better)

// We want to have some allocation logic here, so we can use it to implement libc
WEAK u32 wasmg___heap_base = 0;
u32      runtime_heap_base;

// If a function is registered using the (start) instruction, call it
WEAK void start_fn() {}
WEAK void wasmf__start() {}

extern struct dylib_handler so_handler;
u32                         starting_pages, max_pages;

static inline int dylib_handler_init(struct dylib_handler* handler, const char* path) {
    awsm_assert(handler != NULL);

    int rc = 0;

    handler->handle = dlopen(path, RTLD_LAZY | RTLD_DEEPBIND);
    if (handler->handle == NULL) {
        fprintf(stderr, "Failed to open %s with error: %s\n", path, dlerror());
        goto dl_open_error;
    }

    /* Resolve the symbols in the dynamic library *.so file */
    handler->entrypoint = (entrypoint_fn_t)dlsym(handler->handle, "wasmf__start");
    if (handler->entrypoint == NULL) {
        fprintf(stderr, "Failed to resolve symbol %s in %s with error: %s\n", "wasmf__start", path, dlerror());
        goto dl_error;
    }

    /*
     * This symbol may or may not be present depending on whether the aWsm was
     * run with the --runtime-globals flag. It is not clear what the proper
     * configuration would be for SLEdge, so no validation is performed
     */
    handler->initialize_globals = (init_globals_fn_t)dlsym(handler->handle, "populate_globals");

    handler->initialize_memory = (init_mem_fn_t)dlsym(handler->handle, "populate_memory");
    if (handler->initialize_memory == NULL) {
        fprintf(stderr, "Failed to resolve symbol %s in %s with error: %s\n", "populate_memory", path, dlerror());
        goto dl_error;
    }

    handler->initialize_tables = (init_tbl_fn_t)dlsym(handler->handle, "populate_table");
    if (handler->initialize_tables == NULL) {
        fprintf(stderr, "Failed to resolve symbol %s in %s with error: %s\n", "populate_table", path, dlerror());
        goto dl_error;
    }

    handler->starting_pages = dlsym(handler->handle, "starting_pages");
    handler->max_pages      = dlsym(handler->handle, "max_pages");
    handler->globals_len    = dlsym(handler->handle, "globals_len");
done:
    return rc;
dl_error:
    dlclose(handler->handle);
dl_open_error:
    rc = -1;
    goto done;
}

void runtime_init() {
    char* dl_path = getenv("AWSM_DLSO_PATH");
    if (dl_path == NULL) {
        char dl_path_from_app_path[128];
        strtok(so_handler.app_path, ".");
        sprintf(dl_path_from_app_path, "%s.so", so_handler.app_path);
        dl_path = dl_path_from_app_path;
    }

    int ret = dylib_handler_init(&so_handler, dl_path);
    if (ret != 0)
        exit(ret);

    starting_pages = *so_handler.starting_pages;
    max_pages      = *so_handler.max_pages;

    if (likely(starting_pages > 0)) {
        alloc_linear_memory();
    }

    so_handler.initialize_tables();
    if (so_handler.initialize_globals)
        so_handler.initialize_globals();
    so_handler.initialize_memory();

    int rc = fesetround(FE_TONEAREST);
    awsm_assert(rc == 0);

    runtime_heap_base = wasmg___heap_base;
    if (runtime_heap_base == 0) {
        runtime_heap_base = memory_size;
    }

    start_fn();
}
