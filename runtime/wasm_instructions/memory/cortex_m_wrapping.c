#include "../../runtime.h"

#define MEM_SIZE (WASM_PAGE_SIZE * 1 << 2)
char CORTEX_M_MEM[MEM_SIZE + sizeof(u64)] = { 0 };

INLINE float get_f32(u32 offset) {
    offset = offset % MEM_SIZE;

    void* address = &CORTEX_M_MEM[offset];
    return *(float*)address;
}

INLINE double get_f64(u32 offset) {
    offset = offset % MEM_SIZE;

    void* address = &CORTEX_M_MEM[offset];
    return *(double*)address;
}

INLINE i8 get_i8(u32 offset) {
    offset = offset % MEM_SIZE;

    void* address = &CORTEX_M_MEM[offset];
    return *(i8*)address;
}

INLINE i16 get_i16(u32 offset) {
    return *(i16*)&CORTEX_M_MEM[offset % MEM_SIZE];
}

INLINE i32 get_i32(u32 offset) {
    return *(i32*)&CORTEX_M_MEM[offset % MEM_SIZE];
}

INLINE i64 get_i64(u32 offset) {
    offset = offset % MEM_SIZE;

    void* address = &CORTEX_M_MEM[offset];
    return *(i64*)address;
}

INLINE void set_f32(u32 offset, float v) {
    offset = offset % MEM_SIZE;

    void* address    = &CORTEX_M_MEM[offset];
    *(float*)address = v;
}

INLINE void set_f64(u32 offset, double v) {
    offset = offset % MEM_SIZE;

    void* address     = &CORTEX_M_MEM[offset];
    *(double*)address = v;
}

INLINE void set_i8(u32 offset, i8 v) {
    offset = offset % MEM_SIZE;

    void* address = &CORTEX_M_MEM[offset];
    *(i8*)address = v;
}

INLINE void set_i16(u32 offset, i16 v) {
    offset = offset % MEM_SIZE;

    void* address  = &CORTEX_M_MEM[offset];
    *(i16*)address = v;
}

INLINE void set_i32(u32 offset, i32 v) {
    offset = offset % MEM_SIZE;

    void* address  = &CORTEX_M_MEM[offset];
    *(i32*)address = v;
}

INLINE void set_i64(u32 offset, i64 v) {
    offset = offset % MEM_SIZE;

    void* address  = &CORTEX_M_MEM[offset];
    *(i64*)address = v;
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    awsm_assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    awsm_assert(f.type_id == type_id);
    awsm_assert(f.func_pointer);

    return f.func_pointer;
}
