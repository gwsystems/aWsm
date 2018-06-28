#include <assert.h>
#include <limits.h>
#include <math.h>
#include <printf.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define EXPORT __attribute__ ((visibility ("default")))
#define IMPORT __attribute__ ((visibility ("default")))

// Type alias's so I don't have to write uint32_t a million times
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef __int128 i128;

#define WASM_PAGE_SIZE (1024 * 64)

// The code generator compiles in the starting number of wasm pages, and the maximum number of pages
// If we try and allocate more than max_pages, we should fault
extern u32 starting_pages;
extern u32 max_pages;

// The code generator also compiles in stubs that populate the linear memory and function table
void populate_memory();
void populate_table();

// memory/* provides these memory functions
extern void* memory;
extern u32 memory_size;

void alloc_linear_memory();
void expand_memory();
char* get_memory_ptr(u32 offset, u32 bounds_check);

static void* get_memory_ptr_void(u32 offset, u32 bounds_check) {
    return (void*) get_memory_ptr(offset, bounds_check);
}

// libc/* might need to do some setup for the libc setup
void stub_init(i32 offset);

