#include "../../runtime.h"

INLINE float get_f32(u32 offset) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    return *(float*)address;
}

INLINE double get_f64(u32 offset) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    return *(double*)address;
}

INLINE i8 get_i8(u32 offset) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    return *(i8*)address;
}

INLINE i16 get_i16(u32 offset) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    return *(i16*)address;
}

INLINE i32 get_i32(u32 offset) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    return *(i32*)address;
}

INLINE i64 get_i64(u32 offset) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    return *(i64*)address;
}

INLINE void set_f32(u32 offset, float v) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    *(float*)address   = v;
}

INLINE void set_f64(u32 offset, double v) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    *(double*)address  = v;
}

INLINE void set_i8(u32 offset, i8 v) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    *(i8*)address      = v;
}

INLINE void set_i16(u32 offset, i16 v) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    *(i16*)address     = v;
}

INLINE void set_i32(u32 offset, i32 v) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    *(i32*)address     = v;
}

INLINE void set_i64(u32 offset, i64 v) {
    char* mem_as_chars = (char*)memory;
    void* address      = &mem_as_chars[offset];
    *(i64*)address     = v;
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    awsm_assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    // NOTE: Legacy C applications could fail this check if they typecast function pointers.
    // Additional reference: https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    awsm_assert(f.type_id == type_id);
    awsm_assert(f.func_pointer);

    return f.func_pointer;
}
