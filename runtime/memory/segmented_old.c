#include "../runtime.h"

#include <syscall.h>

#include <asm/ldt.h>
#include <sys/mman.h>

// Routines for dealing with the ldt
void write_ldt(struct user_desc* desc) {
    int i = syscall(__NR_modify_ldt, 1, desc, sizeof(*desc));
    printf("wrote ldt, %d\n", i);
}

#define DS_IDX 10
#define DS_STORAGE_IDX 1000

#define FS_IDX 11
#define FS_STORAGE_IDX 1001

int read_ds() {
    int i;
    asm("movl %%ds, %0" : "=r"(i));
    return i;
}

void write_ds_raw(int v) {
    asm volatile ("movl %0, %%ds" : : "r" (v) : "memory");
}

void write_ds(int idx) {
    int v = (idx << 3) | 0x7;
    write_ds_raw(v);
}

int read_fs() {
    int i;
    asm("movl %%fs, %0" : "=r"(i));
    return i;
}

void write_fs_raw(int v) {
    asm volatile ("movl %0, %%fs" : : "r" (v) : "memory");
}

void write_fs(int idx) {
    int v = (idx << 3) | 0x7;
    write_fs_raw(v);
}

void* memory;
u32 memory_size;

// TODO: Emit a fixed global count, we are vulnerable to overflows as is
#define GLOBAL_COUNT 1024
i32 globals[1024];

INLINE i32 get_global_i32(u32 idx) {
    i32 i;
    asm volatile ("movl %%fs:(%1), %0" : "=r"(i) : "r" (idx) : "memory");
    return i;
}

INLINE void set_global_i32(u32 idx, i32 v) {
    asm volatile ("movl %0, %%fs:(%1)" : : "r" (v), "r" (idx) : "memory");
}

inline static void set_seg_registers() {
    int i = read_fs();
    write_fs(FS_IDX);
    set_global_i32(FS_STORAGE_IDX, (i32) i);

    int j = read_ds();
    write_ds(DS_IDX);
    set_global_i32(DS_STORAGE_IDX, (i32) j);
}

inline static void reset_seg_registers() {
    // Critical that we reset this first, because otherwise global access will break
    int j = get_global_i32(DS_STORAGE_IDX);
    write_ds_raw(j);

    int i = get_global_i32(FS_STORAGE_IDX);
    write_fs_raw(i);
}

void alloc_linear_memory() {
    memory = calloc(starting_pages, WASM_PAGE_SIZE);
    memory_size = starting_pages * WASM_PAGE_SIZE;

    // Setup global access through es
    struct user_desc fs_desc = (struct user_desc) {
        .entry_number = FS_IDX,
        .base_addr = (int) globals,
        .limit = sizeof(globals),
        .seg_32bit = 1, // TODO: Make sure this makes sense
        .contents = 0,
        .read_exec_only = 0,
        .limit_in_pages = 0,
        .seg_not_present = 0,
        .useable = 1,
    };
    write_ldt(&fs_desc);

    struct user_desc ds_desc = (struct user_desc) {
        .entry_number = DS_IDX,
        .base_addr = (int) memory,
        .limit = memory_size / 4096,
        .seg_32bit = 1, // TODO: Make sure this makes sense
        .contents = 0,
        .read_exec_only = 0,
        .limit_in_pages = 1,
        .seg_not_present = 0,
        .useable = 1,
    };
    write_ldt(&ds_desc);
}

void expand_memory() {
    // max_pages = 0 => no limit
    reset_seg_registers();
    assert(max_pages == 0 || (memory_size / WASM_PAGE_SIZE < max_pages));

    memory = realloc(memory, memory_size + WASM_PAGE_SIZE);
    assert(memory);

    char* mem_as_chars = memory;
    memset(&mem_as_chars[memory_size], 0, WASM_PAGE_SIZE);
    memory_size += WASM_PAGE_SIZE;

    struct user_desc ds_desc = (struct user_desc) {
        .entry_number = DS_IDX,
        .base_addr = (int) memory,
        .limit = memory_size / 4096,
        .seg_32bit = 1, // TODO: Make sure this makes sense
        .contents = 0,
        .read_exec_only = 0,
        .limit_in_pages = 1,
        .seg_not_present = 0,
        .useable = 1,
    };
    write_ldt(&ds_desc);

    set_seg_registers();
}

INLINE char* get_memory_ptr(u32 offset, u32 bounds_check) {
//    // There could be a potential bug here if bounds_check > memory_size
//    // But statically we promise that memory_size >= WASM_PAGE_SIZE && bounds_check < WASM_PAGE_SIZE
//    assert(offset <= memory_size - bounds_check);
//
//    char* mem_as_chars = (char *) memory;
//    char* address = &mem_as_chars[offset];
//
    return (char*) offset;
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    assert(f.type_id == type_id && f.func_pointer);

    return f.func_pointer;
}

void switch_into_runtime() {
    reset_seg_registers();
}

void switch_out_of_runtime() {
    set_seg_registers();
}

