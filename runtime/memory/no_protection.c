#include "../runtime.h"

void* memory;
u32 memory_size;

void alloc_linear_memory() {
    for (u32 i = 0; i < starting_pages; i++) {
        expand_memory();
    }
}

void expand_memory() {
    // max_pages = 0 => no limit
    assert(max_pages == 0 || (memory_size / WASM_PAGE_SIZE < max_pages));

    memory = realloc(memory, memory_size + WASM_PAGE_SIZE);
    assert(memory);

    char* mem_as_chars = memory;
    memset(&mem_as_chars[memory_size], 0, WASM_PAGE_SIZE);
    memory_size += WASM_PAGE_SIZE;
}

INLINE char* get_memory_ptr(u32 offset, u32 bounds_check) {
    char* mem_as_chars = (char *) memory;
    return &mem_as_chars[offset];
}
