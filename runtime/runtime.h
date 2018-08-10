#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <printf.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/uio.h>

#define EXPORT __attribute__ ((visibility ("default")))
#define IMPORT __attribute__ ((visibility ("default")))

#define INLINE __attribute__((always_inline))
#define WEAK __attribute__((weak))


// Type alias's so I don't have to write uint32_t a million times
typedef signed char i8;
typedef unsigned char u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

#define WASM_PAGE_SIZE (1024 * 64)

// The code generator compiles in the starting number of wasm pages, and the maximum number of pages
// If we try and allocate more than max_pages, we should fault
extern u32 starting_pages;
extern u32 max_pages;

// Some backends might need to do manual switching when we go into the runtime
INLINE void switch_into_runtime();
INLINE void switch_out_of_runtime();

// The code generator also compiles in stubs that populate the linear memory and function table
void populate_memory();
void populate_table();

// memory/* provides these memory functions
extern void* memory;
extern u32 memory_size;

void alloc_linear_memory();
void expand_memory();
// Assumption: bounds_check < WASM_PAGE_SIZE
INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check);

static inline void* get_memory_ptr_void(u32 offset, u32 bounds_check) {
    return (void*) get_memory_ptr_for_runtime(offset, bounds_check);
}

// memory/* also provides the table access functions
// TODO: Change this to use a compiled in size
#define INDIRECT_TABLE_SIZE 1024

struct indirect_table_entry {
    u32 type_id;
    void* func_pointer;
};

extern struct indirect_table_entry indirect_table[INDIRECT_TABLE_SIZE];

INLINE char* get_function_from_table(u32 idx, u32 type_id);

// libc/* might need to do some setup for the libc setup
void stub_init(i32 offset);

