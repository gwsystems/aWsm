#include "../runtime.h"

#include <syscall.h>

#include <asm/ldt.h>
#include <sys/mman.h>

// Routines for dealing with the ldt
void write_ldt(struct user_desc* desc) {
    int i = syscall(__NR_modify_ldt, 1, desc, sizeof(*desc));
    assert(i == 0);
}

#define GS_IDX 10
int stored_gs_val = 0;

int read_gs() {
    int i;
    asm volatile("movl %%gs, %0" : "=r"(i));
    return i;
}

void write_gs_raw(int v) {
    asm volatile("movl %0, %%gs" : : "r" (v) : "memory");
}

void write_gs(int idx) {
    int v = (idx << 3) | 0x7;
    write_gs_raw(v);
}

void* memory;
u32 memory_size;

inline static void set_seg_registers() {
    stored_gs_val = read_gs();
    write_gs(GS_IDX);
}

inline static void reset_seg_registers() {
    write_gs_raw(stored_gs_val);
}

void alloc_linear_memory() {
    memory = calloc(starting_pages, WASM_PAGE_SIZE);
    memory_size = starting_pages * WASM_PAGE_SIZE;

    struct user_desc gs_desc = (struct user_desc) {
        .entry_number = GS_IDX,
        .base_addr = (int) memory,
        .limit = memory_size / 4096,
        .seg_32bit = 1, // TODO: Make sure this makes sense
        .contents = 0,
        .read_exec_only = 0,
        .limit_in_pages = 1,
        .seg_not_present = 0,
        .useable = 1,
    };
    write_ldt(&gs_desc);
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

    struct user_desc gs_desc = (struct user_desc) {
        .entry_number = GS_IDX,
        .base_addr = (int) memory,
        .limit = memory_size / 4096,
        .seg_32bit = 1, // TODO: Make sure this makes sense
        .contents = 0,
        .read_exec_only = 0,
        .limit_in_pages = 1,
        .seg_not_present = 0,
        .useable = 1,
    };
    write_ldt(&gs_desc);

    set_seg_registers();
}

INLINE char* get_memory_ptr_for_runtime(u32 offset, u32 bounds_check) {
    assert(memory_size > bounds_check && offset <= memory_size - bounds_check);

    char* mem_as_chars = (char *) memory;
    char* address = &mem_as_chars[offset];

    return address;
}

// All of these are pretty generic
INLINE float get_f32(i32 offset) {
    float f;
    asm("movl %%gs:(%1), %0" : "=r"(f) : "r" (offset) : "memory");
    return f;
}

INLINE double get_f64(i32 offset) {
//    i32 a;
//    asm volatile("movl %%gs:(%1), %0" : "=r"(a) : "r" (offset));
//
//    i32 b;
//    asm volatile("movl %%gs:(%1), %0" : "=r"(a) : "r" (offset + sizeof(i32)));
//
//    union {
//        struct {
//            i32 a;
//            i32 b;
//        } s;
//        double f;
//    } u;
//
//    u.s.a = a;
//    u.s.b = b;
//    return u.f;
//    assert(offset <= memory_size - sizeof(i16));
//
//    char* mem_as_chars = (char *) memory;
//    void* address = &mem_as_chars[offset];
//    return *(double *) address;
    double d;
    asm("movq %%gs:(%1), %0" : "=x"(d) : "r" (offset) : "memory");
    return d;
}

INLINE i8 get_i8(i32 offset) {
    assert(offset <= memory_size - sizeof(i8));

    char* mem_as_chars = (char *) memory;
    void* address = &mem_as_chars[offset];
    return *(i8 *) address;
}

INLINE i16 get_i16(i32 offset) {
    assert(offset <= memory_size - sizeof(i16));

    char* mem_as_chars = (char *) memory;
    void* address = &mem_as_chars[offset];
    return *(i16 *) address;
}

INLINE i32 get_i32(i32 offset) {
    i32 i;
    asm("movl %%gs:(%1), %0" : "=r"(i) : "r" (offset) : "memory");
    return i;
}

INLINE i64 get_i64(i32 offset) {
//    i32 a;
//    asm volatile("movl %%gs:(%1), %0" : "=r"(a) : "r" (offset));
//
//    i32 b;
//    asm volatile("movl %%gs:(%1), %0" : "=r"(a) : "r" (offset + sizeof(i32)));
//
//    union {
//        struct {
//            i32 a;
//            i32 b;
//        } s;
//        i64 i;
//    } u;
//    u.s.a = a;
//    u.s.b = b;
//    return u.i;
//    assert(offset <= memory_size - sizeof(i16));
//
//    char* mem_as_chars = (char *) memory;
//    void* address = &mem_as_chars[offset];
//    return *(i64 *) address;
    i64 i;
    asm("movq %%gs:(%1), %0" : "=x"(i) : "r" (offset) : "memory");
    return i;
}

// Now setting routines
INLINE void set_f32(i32 offset, float v) {
    asm("movl %0, %%gs:(%1)" : : "r" (v), "r" (offset) : "memory");
}

INLINE void set_f64(i32 offset, double v) {
//    union {
//        struct {
//            i32 a;
//            i32 b;
//        } s;
//        double f;
//    } u;
//    u.f = v;
//
//    i32 a = u.s.a;
//    asm volatile("movl %0, %%gs:(%1)" : : "r" (a), "r" (offset) : "memory");
//
//    i32 b = u.s.b;
//    asm volatile("movl %0, %%gs:(%1)" : : "r" (b), "r" (offset + sizeof(i32)) : "memory");
//    assert(offset <= memory_size - sizeof(double));
//
//    char* mem_as_chars = (char *) memory;
//    void* address = &mem_as_chars[offset];
//    *(double *) address = v;
    asm("movq %0, %%gs:(%1)" : : "x" (v), "r" (offset) : "memory");
}

INLINE void set_i8(i32 offset, i8 v) {
    asm("movb %0, %%gs:(%1)" : : "r" (v), "r" (offset) : "memory");
}

INLINE void set_i16(i32 offset, i16 v) {
    asm("movw %0, %%gs:(%1)" : : "r" (v), "r" (offset) : "memory");
}

INLINE void set_i32(i32 offset, i32 v) {
    asm("movl %0, %%gs:(%1)" : : "r" (v), "r" (offset) : "memory");
}

INLINE void set_i64(i32 offset, i64 v) {
//    union {
//        struct {
//            i32 a;
//            i32 b;
//        } s;
//        i64 i;
//    } u;
//    u.i = v;
//
//    i32 a = u.s.a;
//    asm volatile("movl %0, %%gs:(%1)" : : "r" (a), "r" (offset) : "memory");
//
//    i32 b = u.s.b;
//    asm volatile("movl %0, %%gs:(%1)" : : "r" (b), "r" (offset + sizeof(i32)) : "memory");
//    assert(offset <= memory_size - sizeof(i16));
//
//    char* mem_as_chars = (char *) memory;
//    void* address = &mem_as_chars[offset];
//    *(i64 *) address = v;
    asm("movq %0, %%gs:(%1)" : : "x" (v), "r" (offset) : "memory");
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    assert(f.type_id == type_id && f.func_pointer);

    return f.func_pointer;
}

INLINE void switch_into_runtime() {
    reset_seg_registers();
}

INLINE void switch_out_of_runtime() {
    set_seg_registers();
}

