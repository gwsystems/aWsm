#include "runtime.h"
#include <fenv.h>
#include <math.h>

struct indirect_table_entry indirect_table[INDIRECT_TABLE_SIZE];

// TODO: Throughout here we use `assert` for error conditions, which isn't optimal
// Instead we should use `unlikely` branches to a single trapping function (which should optimize better)

// Needed to support C++
void env___cxa_pure_virtual() {
    awsm_assert("env___cxa_pure_virtual" == 0);
}

__attribute__((noreturn)) void awsm_abi__trap_unreachable() {
    fprintf(stderr, "WebAssembly control flow unexpectedly reached unreachable instruction\n");
    exit(EXIT_FAILURE);
}

// We want to have some allocation logic here, so we can use it to implement libc
WEAK u32 wasmg___heap_base = 0;
u32      runtime_heap_base;

// If a function is registered using the (start) instruction, call it
WEAK void awsm_abi__start_fn() {}

#ifdef DYNAMIC_LINKING_WASM_SO
void runtime_with_so_init(struct awsm_abi_symbols* abi, char* app_name) {
    char* dl_path = getenv("AWSM_DLSO_PATH");
    if (dl_path == NULL) {
        char app_name_so[128];
        strtok(app_name, ".");
        sprintf(app_name_so, "%s.so", app_name);
        dl_path = app_name_so;
    }

    int ret = awsm_abi_symbols_init(abi, dl_path);
    if (ret != 0)
        exit(ret);

    starting_pages = *abi->starting_pages;
    max_pages      = *abi->max_pages;

    if (likely(starting_pages > 0)) {
        alloc_linear_memory();
    }

    abi->initialize_tables();
    if (abi->initialize_globals)
        abi->initialize_globals();
    abi->initialize_memory();

    awsm_abi__start_fn();
}

#else

void runtime_init() {
    if (likely(starting_pages > 0)) {
        alloc_linear_memory();
    }

    populate_table();
    switch_out_of_runtime();
    populate_globals();
    switch_into_runtime();
    populate_memory();

    // Rounding always is round-to-nearest ties-to-even, in correspondence with IEEE 754-2019
    // https://webassembly.github.io/spec/core/exec/numerics.html#floating-point-operations
    int rc = fesetround(FE_TONEAREST);
    awsm_assert(rc == 0);

    runtime_heap_base = wasmg___heap_base;
    if (runtime_heap_base == 0) {
        runtime_heap_base = memory_size;
    }

    // The start component of a module declares the function index of a start function that is automatically invoked
    // when the module is instantiated, after tables and memories have been initialized.
    // https://webassembly.github.io/spec/core/syntax/modules.html#start-function
    awsm_abi__start_fn();
}

#endif
