#include "../runtime.h"

#include <asm/ldt.h>
#include <sys/mman.h>
#include <syscall.h>

// Routines for dealing with the ldt
void
write_ldt(struct user_desc *desc)
{
	int i = syscall(__NR_modify_ldt, 1, desc, sizeof(*desc));
	awsm_assert(i == 0);
}

#define GS_IDX 10
int stored_gs_val = 0;

int
read_gs()
{
	int i;
	asm volatile("movl %%gs, %0" : "=r"(i));
	return i;
}

void
write_gs_raw(int v)
{
	asm volatile("movl %0, %%gs" : : "r"(v) : "memory");
}

void
write_gs(int idx)
{
	int v = (idx << 3) | 0x7;
	write_gs_raw(v);
}

void *memory;
u32   memory_size;

inline static void
set_seg_registers()
{
	stored_gs_val = read_gs();
	write_gs(GS_IDX);
}

inline static void
reset_seg_registers()
{
	write_gs_raw(stored_gs_val);
}

void
alloc_linear_memory()
{
	awsm_assert(starting_pages > 0);
	awsm_assert(max_pages > 0);

	memory      = calloc(starting_pages, WASM_PAGE_SIZE);
	memory_size = starting_pages * WASM_PAGE_SIZE;

	struct user_desc gs_desc = (struct user_desc){
	  .entry_number    = GS_IDX,
	  .base_addr       = (int)memory,
	  .limit           = memory_size / 4096,
	  .seg_32bit       = 1, // TODO: Make sure this makes sense
	  .contents        = 0,
	  .read_exec_only  = 0,
	  .limit_in_pages  = 1,
	  .seg_not_present = 0,
	  .useable         = 1,
	};
	write_ldt(&gs_desc);
}

void
expand_memory()
{
	reset_seg_registers();
	awsm_assert(memory_size / WASM_PAGE_SIZE < max_pages);

	memory = realloc(memory, memory_size + WASM_PAGE_SIZE);
	awsm_assert(memory);

	char *mem_as_chars = memory;
	memset(&mem_as_chars[memory_size], 0, WASM_PAGE_SIZE);
	memory_size += WASM_PAGE_SIZE;

	struct user_desc gs_desc = (struct user_desc){
	  .entry_number    = GS_IDX,
	  .base_addr       = (int)memory,
	  .limit           = memory_size / 4096,
	  .seg_32bit       = 1, // TODO: Make sure this makes sense
	  .contents        = 0,
	  .read_exec_only  = 0,
	  .limit_in_pages  = 1,
	  .seg_not_present = 0,
	  .useable         = 1,
	};
	write_ldt(&gs_desc);

	set_seg_registers();
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
	char *address      = &mem_as_chars[offset];

	return address;
}

INLINE void
switch_into_runtime()
{
	reset_seg_registers();
}

INLINE void
switch_out_of_runtime()
{
	set_seg_registers();
}
