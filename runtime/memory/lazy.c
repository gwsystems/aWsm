#include "../runtime.h"

#ifndef WASM_BLOCK_SIZE
#define WASM_BLOCK_SIZE 4096
#endif

void **memory_blocks;
u32 memory_size;

void alloc_linear_memory() {
    memory_blocks = malloc((starting_pages * WASM_PAGE_SIZE / WASM_BLOCK_SIZE)
            * sizeof(void*));
    memset(memory_blocks, 0, (starting_pages * WASM_PAGE_SIZE / WASM_BLOCK_SIZE)
            * sizeof(void*));
    memory_size = starting_pages * WASM_PAGE_SIZE;
    printf("mem size: %d\n", memory_size);
}

void expand_memory() {
    printf("mem size: %d -> %d\n", memory_size, memory_size + WASM_PAGE_SIZE);
    // max_pages = 0 => no limit
    assert(max_pages == 0 || (memory_size / WASM_PAGE_SIZE < max_pages));

    memory_blocks = realloc(memory_blocks,
            ((memory_size + WASM_PAGE_SIZE) / WASM_BLOCK_SIZE) * sizeof(void*));
    assert(memory_blocks);

    memory_blocks[memory_size / WASM_BLOCK_SIZE] = NULL;
    memory_size += WASM_PAGE_SIZE;
}

// FIXME: This can't be provided by this memory backend
//INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check) {
//    assert(memory_size > bounds_check && offset <= memory_size - bounds_check);
//
//    char* mem_as_chars = (char *) memory;
//    char* address = &mem_as_chars[offset];
//
//    return address;
//}

// All of these are pretty generic
INLINE i8 get_i8(i32 offset) {
    assert(offset <= memory_size - sizeof(i8));

    void *block = memory_blocks[offset / WASM_BLOCK_SIZE];
    if (!block) {
        return 0;
    }

    return ((i8*)block)[offset % WASM_BLOCK_SIZE];
}

INLINE i16 get_i16(i32 offset) {
    return ((u16)(u8)get_i8(offset+0) << 0) |
           ((u16)(u8)get_i8(offset+1) << 8);
}

INLINE i32 get_i32(i32 offset) {
    return ((u32)(u8)get_i8(offset+0) << 0) |
           ((u32)(u8)get_i8(offset+1) << 8) |
           ((u32)(u8)get_i8(offset+2) << 16) |
           ((u32)(u8)get_i8(offset+3) << 24);
}

INLINE i64 get_i64(i32 offset) {
    return ((u64)(u8)get_i8(offset+0) << 0) |
           ((u64)(u8)get_i8(offset+1) << 8) |
           ((u64)(u8)get_i8(offset+2) << 16) |
           ((u64)(u8)get_i8(offset+3) << 24) |
           ((u64)(u8)get_i8(offset+4) << 32) |
           ((u64)(u8)get_i8(offset+5) << 40) |
           ((u64)(u8)get_i8(offset+6) << 48) |
           ((u64)(u8)get_i8(offset+7) << 56);
}

INLINE float get_f32(i32 offset) {
    i32 x = get_i32(offset);
    return *(float*)&x;
}

INLINE double get_f64(i32 offset) {
    i64 x = get_i32(offset);
    return *(double*)&x;
}


// Now setting routines
INLINE void set_i8(i32 offset, i8 v) {
    assert(offset <= memory_size - sizeof(i8));

    if (!memory_blocks[offset / WASM_BLOCK_SIZE]) {
        memory_blocks[offset / WASM_BLOCK_SIZE] = malloc(WASM_BLOCK_SIZE);
        memset(memory_blocks[offset / WASM_BLOCK_SIZE], 0, WASM_BLOCK_SIZE);
    }

    ((i8**)memory_blocks)[offset / WASM_BLOCK_SIZE][offset % WASM_BLOCK_SIZE] = v;
}

INLINE void set_i16(i32 offset, i16 v) {
    set_i8(offset+0, v >> 0);
    set_i8(offset+1, v >> 8);
}

INLINE void set_i32(i32 offset, i32 v) {
    set_i8(offset+0, v >> 0);
    set_i8(offset+1, v >> 8);
    set_i8(offset+2, v >> 16);
    set_i8(offset+3, v >> 24);
}

INLINE void set_i64(i32 offset, i64 v) {
    set_i8(offset+0, v >> 0);
    set_i8(offset+1, v >> 8);
    set_i8(offset+2, v >> 16);
    set_i8(offset+3, v >> 24);
    set_i8(offset+4, v >> 32);
    set_i8(offset+5, v >> 40);
    set_i8(offset+6, v >> 48);
    set_i8(offset+7, v >> 56);
}

INLINE void set_f32(i32 offset, float v) {
    set_i32(offset, *(i32*)&v);
}

INLINE void set_f64(i32 offset, double v) {
    set_i64(offset, *(i64*)&v);
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    assert(f.type_id == type_id && f.func_pointer);

    return f.func_pointer;
}

// Functions that aren't useful for this runtime
INLINE void switch_into_runtime() {}
INLINE void switch_out_of_runtime() {}
