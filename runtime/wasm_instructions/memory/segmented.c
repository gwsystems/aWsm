#include "../../runtime.h"

#define GS_REL __attribute__((address_space(256)))

INLINE float get_f32(u32 offset) {
    return *((GS_REL float*)offset);
}

INLINE double get_f64(u32 offset) {
    return *((GS_REL double*)offset);
}

INLINE i8 get_i8(u32 offset) {
    return *((GS_REL i8*)offset);
}

INLINE i16 get_i16(u32 offset) {
    return *((GS_REL i16*)offset);
}

INLINE i32 get_i32(u32 offset) {
    return *((GS_REL i32*)offset);
}

INLINE i64 get_i64(u32 offset) {
    return *((GS_REL i64*)offset);
}

INLINE void set_f32(u32 offset, float v) {
    GS_REL float* ptr = (GS_REL float*)offset;
    *ptr              = v;
}

INLINE void set_f64(u32 offset, double v) {
    GS_REL double* ptr = (GS_REL double*)offset;
    *ptr               = v;
}

INLINE void set_i8(u32 offset, i8 v) {
    GS_REL i8* ptr = (GS_REL i8*)offset;
    *ptr           = v;
}

INLINE void set_i16(u32 offset, i16 v) {
    GS_REL i16* ptr = (GS_REL i16*)offset;
    *ptr            = v;
}

INLINE void set_i32(u32 offset, i32 v) {
    GS_REL i32* ptr = (GS_REL i32*)offset;
    *ptr            = v;
}

INLINE void set_i64(u32 offset, i64 v) {
    GS_REL i64* ptr = (GS_REL i64*)offset;
    *ptr            = v;
}

INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    awsm_assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    awsm_assert(f.type_id == type_id);
    awsm_assert(f.func_pointer);

    return f.func_pointer;
}
