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

// We want to have some allocation logic here, so we can use it to implement libc
WEAK u32 wasmg___heap_base = 0;
u32      runtime_heap_base;

// If a function is registered using the (start) instruction, call it
WEAK void awsm_abi__start_fn() {}

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
