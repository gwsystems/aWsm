#include "../runtime.h"

u32 memory_size = 0;

#define MEM_SIZE (WASM_PAGE_SIZE * 1 << 2)
char CORTEX_M_MEM[MEM_SIZE + sizeof(u64)] = { 0 };

u32 cortex_mem_size = MEM_SIZE;

int printf_(const char* format, ...);

void alloc_linear_memory() {
    awsm_assert(starting_pages > 0);
    awsm_assert(max_pages > 0);

    //    printf_("8 = (%d %d) 16 = (%d %d) 32 = (%d %d) 64 = (%d %d)\n", sizeof(u8), sizeof(i8), sizeof(u16),
    //    sizeof(i16), sizeof(u32), sizeof(i32), sizeof(u64), sizeof(i64)); printf_("starting pages = %d, starting
    //    mem size = %d\n", starting_pages, starting_pages * WASM_PAGE_SIZE);
    awsm_assert(MEM_SIZE >= starting_pages * WASM_PAGE_SIZE);
    memory_size = starting_pages * WASM_PAGE_SIZE;
}

void expand_memory() {
    awsm_assert(memory_size / WASM_PAGE_SIZE < max_pages);

    //    printf_("Expanding to %d\n", memory_size + WASM_PAGE_SIZE);
    awsm_assert(memory_size + WASM_PAGE_SIZE <= sizeof(CORTEX_M_MEM));

    memset(&CORTEX_M_MEM[memory_size], 0, WASM_PAGE_SIZE);
    memory_size += WASM_PAGE_SIZE;
}

INLINE void check_bounds(u32 offset, u32 bounds_check) {
    awsm_assert(offset <= memory_size - bounds_check);
}

INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check) {
    check_bounds(offset, bounds_check);

    char* address = &CORTEX_M_MEM[offset];
    return address;
}

// Functions that aren't useful for this runtime
INLINE void switch_into_runtime() {}
INLINE void switch_out_of_runtime() {}
