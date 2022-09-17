#include "../../runtime.h"

#define SPT_PAGE_COUNT_ORDER 8
#define SPT_PAGE_COUNT       (1 << SPT_PAGE_COUNT_ORDER)

#define SPT_PAGE_SIZE_ORDER 10
#define SPT_PAGE_SIZE       (1 << SPT_PAGE_SIZE_ORDER)

static void* PAGE_TABLE[SPT_PAGE_COUNT] = { 0 };

INLINE void* get_page(u32 offset) {
    u32 page_number = (offset >> SPT_PAGE_SIZE_ORDER) & ((1 << SPT_PAGE_COUNT_ORDER) - 1);
    awsm_assert(page_number < SPT_PAGE_COUNT);
    void* page = PAGE_TABLE[page_number];
    awsm_assert(page);
    return page;
}

INLINE u32 get_page_offset(u32 offset) {
    return offset & ((1 << SPT_PAGE_SIZE_ORDER) - 1);
}

INLINE i8 get_i8(u32 offset) {
    i8* page        = get_page(offset);
    u32 page_offset = get_page_offset(offset);
    return page[page_offset];
}

INLINE i16 get_i16(u32 offset) {
    u32 page_offset = get_page_offset(offset);
    if (page_offset < SPT_PAGE_SIZE - sizeof(i16)) {
        i8*  page     = get_page(offset);
        i16* page_adj = (void*)&page[page_offset];
        return *page_adj;
    } else {
        u8 a = (u8)get_i8(offset);
        u8 b = (u8)get_i8(offset + 1);
        return (i16)(((u16)b) << 8) | ((u16)a);
    }
    //    u32 page_offset = get_page_offset(offset);
    //    i8* page = get_page(offset);
    //    i16* page_adj = (void*) &page[page_offset];
    //    printf("Regularly loaded: %p\n", (void*) (u64) *page_adj);
    //
    //    u8 a = (u8) get_i8(offset);
    //    u8 b = (u8) get_i8(offset + 1);
    //    i16 split = (i16) (((u16) b) << 8) | ((u16) a);
    //    printf("Split from (%p, %p)\n", (void*) (u64) a, (void*) (u64) b);
    //    printf("Split loaded: %p\n", (void*) (u64) split);
    //
    //    awsm_assert(*page_adj == split);
    //    return *page_adj;
}

INLINE i32 get_i32(u32 offset) {
    u32 page_offset = get_page_offset(offset);
    if (page_offset < SPT_PAGE_SIZE - sizeof(i32)) {
        i8*  page     = get_page(offset);
        i32* page_adj = (void*)&page[page_offset];
        return *page_adj;
    } else {
        u16 a = (u16)get_i16(offset);
        u16 b = (u16)get_i16(offset + 2);
        return (i32)(((u32)b) << 16) | ((u32)a);
    }
}

INLINE i64 get_i64(u32 offset) {
    u32 page_offset = get_page_offset(offset);
    if (page_offset < SPT_PAGE_SIZE - sizeof(i64)) {
        i8*  page     = get_page(offset);
        i64* page_adj = (void*)&page[page_offset];
        return *page_adj;
    } else {
        u32 a = (u32)get_i32(offset);
        u32 b = (u32)get_i32(offset + 4);
        return (i64)(((u64)b) << 32) | ((u64)a);
    }
}

INLINE float get_f32(u32 offset) {
    union {
        i32   i;
        float f;
    } a;

    a.i = get_i32(offset);
    return a.f;
}

INLINE double get_f64(u32 offset) {
    union {
        i64    i;
        double f;
    } a;

    a.i = get_i64(offset);
    return a.f;
}

// Setting routines
INLINE void set_i8(u32 offset, i8 v) {
    i8* page          = get_page(offset);
    u32 page_offset   = get_page_offset(offset);
    page[page_offset] = v;
}

INLINE void set_i16(u32 offset, i16 v) {
    u32 page_offset = get_page_offset(offset);
    if (page_offset < SPT_PAGE_SIZE - sizeof(i16)) {
        i8*  page     = get_page(offset);
        i16* page_adj = (void*)&page[page_offset];
        *page_adj     = v;
    } else {
        u16 v2 = (u16)v;

        set_i8(offset, (i8)v2);
        set_i8(offset + 1, (i8)(v2 >> 8));
    }
}

INLINE void set_i32(u32 offset, i32 v) {
    u32 page_offset = get_page_offset(offset);
    if (page_offset < SPT_PAGE_SIZE - sizeof(i32)) {
        i8*  page     = get_page(offset);
        i32* page_adj = (void*)&page[page_offset];
        *page_adj     = v;
    } else {
        u32 v2 = (u32)v;

        set_i16(offset, (i16)v2);
        set_i16(offset + 2, (i16)(v2 >> 16));
    }
}

INLINE void set_i64(u32 offset, i64 v) {
    u32 page_offset = get_page_offset(offset);
    if (page_offset < SPT_PAGE_SIZE - sizeof(i64)) {
        i8*  page     = get_page(offset);
        i64* page_adj = (void*)&page[page_offset];
        *page_adj     = v;
    } else {
        u64 v2 = (u64)v;

        set_i32(offset, (i32)v2);
        set_i32(offset + 4, (i32)(v2 >> 32));
    }
}

INLINE void set_f32(u32 offset, float v) {
    union {
        i32   i;
        float f;
    } a;

    a.f = v;
    set_i32(offset, a.i);
}

INLINE void set_f64(u32 offset, double v) {
    union {
        i64    i;
        double f;
    } a;

    a.f = v;
    set_i64(offset, a.i);
}


INLINE char* get_function_from_table(u32 idx, u32 type_id) {
    awsm_assert(idx < INDIRECT_TABLE_SIZE);

    struct indirect_table_entry f = indirect_table[idx];

    awsm_assert(f.type_id == type_id);
    awsm_assert(f.func_pointer);

    return f.func_pointer;
}
