#include "../runtime.h"

void* memory;
u32   memory_size;

void alloc_linear_memory() {
    awsm_assert(starting_pages > 0);
    awsm_assert(max_pages > 0);

    memory      = calloc(starting_pages, WASM_PAGE_SIZE);
    memory_size = starting_pages * WASM_PAGE_SIZE;
}

void expand_memory() {
    awsm_assert(memory_size / WASM_PAGE_SIZE < max_pages);

    printf("Expanding to %d\n", memory_size + WASM_PAGE_SIZE);

    memory = realloc(memory, memory_size + WASM_PAGE_SIZE);
    awsm_assert(memory);

    char* mem_as_chars = memory;
    memset(&mem_as_chars[memory_size], 0, WASM_PAGE_SIZE);
    memory_size += WASM_PAGE_SIZE;
}

INLINE void check_bounds(u32 offset, u32 bounds_check) {
    awsm_assert(memory_size > bounds_check && offset <= memory_size - bounds_check);
}

INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check) {
    check_bounds(offset, bounds_check);

    char* mem_as_chars = (char*)memory;
    char* address      = &mem_as_chars[offset];

    return address;
}

// Functions that aren't useful for this runtime
INLINE void switch_into_runtime() {}
INLINE void switch_out_of_runtime() {}
