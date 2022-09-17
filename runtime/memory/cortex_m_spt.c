#include "../runtime.h"

void* memory;
u32   memory_size;

int printf_(const char* format, ...);

// Each page is
#define SPT_PAGE_COUNT_ORDER 8
#define SPT_PAGE_COUNT       (1 << SPT_PAGE_COUNT_ORDER)

#define SPT_PAGE_SIZE_ORDER 10
#define SPT_PAGE_SIZE       (1 << SPT_PAGE_SIZE_ORDER)

static void* PAGE_TABLE[SPT_PAGE_COUNT] = { 0 };

#define MEM_SIZE    ((SPT_PAGE_COUNT + 1) * SPT_PAGE_SIZE)
#define TOTAL_PAGES (MEM_SIZE / WASM_PAGE_SIZE)

char CORTEX_M_MEM[MEM_SIZE] = { 0 };

u32 cortex_mem_size = MEM_SIZE;

void alloc_linear_memory() {
    awsm_assert(starting_pages > 0);
    awsm_assert(max_pages > 0);

    //    printf_("8 = (%d %d) 16 = (%d %d) 32 = (%d %d) 64 = (%d %d)\n", sizeof(u8), sizeof(i8), sizeof(u16),
    //    sizeof(i16), sizeof(u32), sizeof(i32), sizeof(u64), sizeof(i64));
    awsm_assert(TOTAL_PAGES >= starting_pages);

    memory      = &CORTEX_M_MEM[0];
    memory_size = starting_pages * WASM_PAGE_SIZE;

    for (int i = 0; i < SPT_PAGE_COUNT; i++) {
        PAGE_TABLE[i] = &CORTEX_M_MEM[i * SPT_PAGE_SIZE];
    }
}

void expand_memory() {
    awsm_assert(memory_size / WASM_PAGE_SIZE < max_pages);
    awsm_assert(memory_size + WASM_PAGE_SIZE <= sizeof(CORTEX_M_MEM));

    char* mem_as_chars = memory;
    memset(&mem_as_chars[memory_size], 0, WASM_PAGE_SIZE);
    memory_size += WASM_PAGE_SIZE;
}

INLINE void check_bounds(u32 offset, u32 bounds_check) {
    return;
}

INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check) {
    char* mem_as_chars = (char*)memory;
    char* address      = &mem_as_chars[offset];

    return address;
}

// Functions that aren't useful for this runtime
INLINE void switch_into_runtime() {}
INLINE void switch_out_of_runtime() {}
