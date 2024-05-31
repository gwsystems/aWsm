#include "../runtime.h"

void *memory;
u32   memory_size;

void
alloc_linear_memory()
{
	awsm_assert(starting_pages > 0);
	awsm_assert(max_pages > 0);

	for (u32 i = 0; i < starting_pages; i++) { expand_memory(); }

	asm volatile("bndmk (%0,%1,1), %%bnd0" : : "r"(memory), "r"((intptr_t)memory_size));
}

void
expand_memory()
{
	awsm_assert(memory_size / WASM_PAGE_SIZE < max_pages);

	memory = realloc(memory, memory_size + WASM_PAGE_SIZE);
	awsm_assert(memory);

	char *mem_as_chars = memory;
	memset(&mem_as_chars[memory_size], 0, WASM_PAGE_SIZE);
	memory_size += WASM_PAGE_SIZE;
}

INLINE void
check_bounds(u32 offset, u32 bounds_check)
{
	awsm_assert(memory_size > bounds_check && offset <= memory_size - bounds_check);
}

INLINE char *
get_memory_ptr_for_runtime(u32 offset, u32 bounds_check)
{
	check_bounds(offset, bounds_check);

	char *mem_as_chars = (char *)memory;
	return &mem_as_chars[offset];
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
