#include "../runtime.h"

#include <sys/mman.h>

void* memory = NULL;
u32 memory_size = 0;

void alloc_linear_memory() {
    // Map 4gb + PAGE_SIZE of memory that will fault when accessed
    // We allocate the extra page so that reads off the end will also fail
    memory = mmap(NULL, (1LL << 32) + WASM_PAGE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("Mapping of initial unusable region failed");
        exit(1);
    }

    for (u32 i = 0; i < starting_pages; i++) {
        expand_memory();
    }
}

void expand_memory() {
    // max_pages = 0 => no limit
    assert(max_pages == 0 || (memory_size / WASM_PAGE_SIZE < max_pages));
    // Remap the relevant wasm page to readable
    char* mem_as_chars = memory;
    char* page_address = &mem_as_chars[memory_size];

    void* map_result = mmap(page_address, WASM_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (map_result == MAP_FAILED) {
        perror("Mapping of new memory failed");
        exit(1);
    }
    memory_size += WASM_PAGE_SIZE;
}

char* get_memory_ptr(u32 offset, u32 bounds_check) {
    // Due to how we setup memory for x86, the virtual memory mechanism will catch the error
    char* mem_as_chars = (char *) memory;
    char* address = &mem_as_chars[offset];
    return address;
}
