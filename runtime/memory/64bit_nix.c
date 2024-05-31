#include "../runtime.h"

#include <sys/mman.h>

void *memory      = NULL;
u32   memory_size = 0;

void
alloc_linear_memory()
{
	awsm_assert(starting_pages > 0);
	awsm_assert(max_pages > 0);

	// Map 4gb + PAGE_SIZE of memory that will fault when accessed
	// We allocate the extra page so that reads off the end will also fail
	memory = mmap(NULL, (1LL << 32) + WASM_PAGE_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == MAP_FAILED) {
		perror("Mapping of initial unusable region failed");
		exit(1);
	}

	void *map_result = mmap(memory, WASM_PAGE_SIZE * starting_pages, PROT_READ | PROT_WRITE,
	                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (map_result == MAP_FAILED) {
		perror("Mapping of initial memory failed");
		exit(1);
	}
	memory_size = WASM_PAGE_SIZE * starting_pages;
}

void
expand_memory()
{
	awsm_assert(memory_size / WASM_PAGE_SIZE < max_pages);
	// Remap the relevant wasm page to readable
	char *mem_as_chars = memory;
	char *page_address = &mem_as_chars[memory_size];

	void *map_result = mmap(page_address, WASM_PAGE_SIZE, PROT_READ | PROT_WRITE,
	                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (map_result == MAP_FAILED) {
		perror("Mapping of new memory failed");
		exit(1);
	}
	memory_size += WASM_PAGE_SIZE;
}

INLINE void
check_bounds(u32 offset, u32 bounds_check)
{
	// Due to how we setup memory for x86, the virtual memory mechanism will catch the error, if bounds <
	// WASM_PAGE_SIZE
	awsm_assert(bounds_check < WASM_PAGE_SIZE
	            || (memory_size > bounds_check && offset <= memory_size - bounds_check));
}

INLINE char *
get_memory_ptr_for_runtime(u32 offset, u32 bounds_check)
{
	check_bounds(offset, bounds_check);

	char *mem_as_chars = (char *)memory;
	char *address      = &mem_as_chars[offset];
	return address;
}

// Functions that aren't useful for this runtime
INLINE void
switch_into_runtime()
{
}
INLINE void
switch_out_of_runtime()
{
}
