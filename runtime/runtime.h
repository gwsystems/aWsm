#pragma once

#include <dlfcn.h>

#define EXPORT __attribute__((visibility("default")))
#define IMPORT __attribute__((visibility("default")))

#define INLINE __attribute__((always_inline))
#define WEAK   __attribute__((weak))

#define likely(X)   __builtin_expect(!!(X), 1)
#define unlikely(X) __builtin_expect(!!(X), 0)

#if __has_include("assert.h") && (defined(__APPLE__) || defined(__linux__))
#include <assert.h>
#define awsm_assert assert
#else
int printf_(const char* format, ...);
#define awsm_assert(x)             \
    do {                           \
        if (!(x)) {                \
            char msg[] = "" #x ""; \
            printf_("%s\n", msg);  \
            while (1)              \
                ;                  \
        }                          \
    } while (0);
#endif

// Type alias's so I don't have to write uint32_t a million times
typedef signed char   i8;
typedef unsigned char u8;

#if __has_include("stdint.h")
#include <stdint.h>
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
#else
// FIXME: Cortex-m specific hack
typedef signed short       i16;
typedef unsigned short     u16;
typedef signed int         i32;
typedef unsigned int       u32;
typedef signed long long   i64;
typedef unsigned long long u64;
#endif

#if __has_include("string.h") && __has_include("math.h") && __has_include("stdio.h") && __has_include("stdlib.h")
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else

#define size_t u32

#define CORTEX_M

void*  memcpy(void* dest, const void* src, size_t len);
void*  memset(void* s, int c, size_t n);
char*  strcpy(char* dest, const char* src);
size_t strlen(const char* str);

double trunc(double x);
float  truncf(float x);

#endif

#if __has_include("limits.h")
#include <limits.h>
#else
#define CHAR_BIT 8

#define INT8_MIN  (-1 - 0x7f)
#define INT16_MIN (-1 - 0x7fff)
#define INT32_MIN (-1 - 0x7fffffff)
#define INT64_MIN (-1 - 0x7fffffffffffffff)

#define INT8_MAX  (0x7f)
#define INT16_MAX (0x7fff)
#define INT32_MAX (0x7fffffff)
#define INT64_MAX (0x7fffffffffffffff)

#define UINT8_MAX  (0xff)
#define UINT16_MAX (0xffff)
#define UINT32_MAX (0xffffffff)
#define UINT64_MAX (0xffffffffffffffff)
#endif

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
void populate_globals();

// memory/* provides these memory functions
extern void* memory;
extern u32   memory_size;
extern u32   runtime_heap_base;

void         alloc_linear_memory();
void         expand_memory();
INLINE void  check_bounds(u32 offset, u32 bounds_check);
INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check);

INLINE i8   get_i8(u32 offset);
INLINE i16  get_i16(u32 offset);
INLINE i32  get_i32(u32 offset);
INLINE i64  get_i64(u32 offset);
INLINE void set_i8(u32 offset, i8 v);
INLINE void set_i16(u32 offset, i16 v);
INLINE void set_i32(u32 offset, i32 v);
INLINE void set_i64(u32 offset, i64 v);

INLINE float  get_f32(u32 offset);
INLINE double get_f64(u32 offset);
INLINE void   set_f32(u32 offset, float);
INLINE void   set_f64(u32 offset, double);

i32 instruction_memory_size();
i32 instruction_memory_grow();

static inline void* get_memory_ptr_void(u32 offset, u32 bounds_check) {
    return (void*)get_memory_ptr_for_runtime(offset, bounds_check);
}

static inline char* get_memory_string(u32 offset) {
    char* naive_ptr = get_memory_ptr_for_runtime(offset, 1);
    int   i         = 0;
    while (1) {
        // Keep bounds checking the waters over and over until we know it's safe (we find a terminating
        // character)
        char ith_element = get_memory_ptr_for_runtime(offset, i + 1)[i];
        if (ith_element == '\0') {
            return naive_ptr;
        }
        i++;
    }
}

u32 allocate_n_bytes(u32 n);

// memory/* also provides the table access functions
// TODO: Change this to use a compiled in size
#define INDIRECT_TABLE_SIZE 1024

struct indirect_table_entry {
    u32   type_id;
    void* func_pointer;
};

extern struct indirect_table_entry indirect_table[INDIRECT_TABLE_SIZE];

INLINE char* get_function_from_table(u32 idx, u32 type_id);

// libc/* might need to do some setup for the libc setup
void stub_init();

// The runtime entrypoint must be called
int runtime_main(int argc, char** argv);

#ifdef DYNAMIC_LINKING_WASM_SO

typedef void (*awsm_abi__init_globals_fn_t)(void);
typedef void (*awsm_abi__init_mem_fn_t)(void);
typedef void (*awsm_abi__init_tbl_fn_t)(void);
typedef int32_t (*awsm_abi__entrypoint_fn_t)(void);
typedef uint32_t (*awsm_abi__wasm_memory_starting_pages_fn_t)(void);
typedef uint32_t (*awsm_abi__wasm_memory_max_pages_fn_t)(void);
typedef uint32_t (*awsm_abi__wasm_memory_globals_len_fn_t)(void);

struct awsm_abi_symbols {
    void*                       handle;
    awsm_abi__init_globals_fn_t initialize_globals;
    awsm_abi__init_mem_fn_t     initialize_memory;
    awsm_abi__init_tbl_fn_t     initialize_tables;
    awsm_abi__entrypoint_fn_t   entrypoint;
    uint32_t*                   starting_pages;
    uint32_t*                   max_pages;
    uint32_t*                   globals_len;
};

static inline int awsm_abi_symbols_init(struct awsm_abi_symbols* abi, const char* path) {
    awsm_assert(abi != NULL);

    int rc = 0;

    abi->handle = dlopen(path, RTLD_LAZY | RTLD_DEEPBIND);
    if (abi->handle == NULL) {
        fprintf(stderr, "Failed to open %s with error: %s\n", path, dlerror());
        goto dl_open_error;
    };

    /* Resolve the symbols in the dynamic library *.so file */
    abi->entrypoint = (awsm_abi__entrypoint_fn_t)dlsym(abi->handle, "wasmf__start");
    if (abi->entrypoint == NULL) {
        fprintf(stderr, "Failed to resolve symbol %s in %s with error: %s\n", "wasmf__start", path, dlerror());
        goto dl_error;
    }

    /*
     * This symbol may or may not be present depending on whether the aWsm was
     * run with the --runtime-globals flag. It is not clear what the proper
     * configuration would be for SLEdge, so no validation is performed
     */
    abi->initialize_globals = (awsm_abi__init_globals_fn_t)dlsym(abi->handle, "populate_globals");

    abi->initialize_memory = (awsm_abi__init_mem_fn_t)dlsym(abi->handle, "populate_memory");
    if (abi->initialize_memory == NULL) {
        fprintf(stderr, "Failed to resolve symbol %s in %s with error: %s\n", "populate_memory", path, dlerror());
        goto dl_error;
    }

    abi->initialize_tables = (awsm_abi__init_tbl_fn_t)dlsym(abi->handle, "populate_table");
    if (abi->initialize_tables == NULL) {
        fprintf(stderr, "Failed to resolve symbol %s in %s with error: %s\n", "populate_table", path, dlerror());
        goto dl_error;
    }

    abi->starting_pages = dlsym(abi->handle, "starting_pages");
    abi->max_pages = dlsym(abi->handle, "max_pages");
    abi->globals_len = dlsym(abi->handle, "globals_len");
done:
    return rc;
dl_error:
    dlclose(abi->handle);
dl_open_error:
    rc = -1;
    goto done;
}

void runtime_with_so_init(struct awsm_abi_symbols*, char *);

#else

void runtime_init();

#endif

