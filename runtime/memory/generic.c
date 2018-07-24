#include "../runtime.h"

void* memory;
u32 memory_size;

void alloc_linear_memory() {
    memory = calloc(starting_pages, WASM_PAGE_SIZE);
    memory_size = starting_pages * WASM_PAGE_SIZE;
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
    // There could be a potential bug here if bounds_check > memory_size
    // But statically we promise that memory_size >= WASM_PAGE_SIZE && bounds_check < WASM_PAGE_SIZE
    assert(offset <= memory_size - bounds_check);

    char* mem_as_chars = (char *) memory;
    char* address = &mem_as_chars[offset];

    return address;
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    assert(f.type_id == type_id && f.func_pointer);

    return f.func_pointer;
}