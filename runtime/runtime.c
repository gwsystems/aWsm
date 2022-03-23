#include "runtime.h"
#include <fenv.h>
#include <math.h>

// TODO: Throughout here we use `assert` for error conditions, which isn't optimal
// Instead we should use `unlikely` branches to a single trapping function (which should optimize better)

// Needed to support C++
void env___cxa_pure_virtual() {
    awsm_assert("env___cxa_pure_virtual" == 0);
}

// Region initialization helper function
EXPORT void initialize_region(u32 offset, u32 data_count, char* data) {
    awsm_assert(memory_size >= data_count);
    awsm_assert(offset < memory_size - data_count);
    //    awsm_assert(offset <= memory_size - data_count);

    // FIXME: Hack around segmented and unsegmented access
    memcpy(get_memory_ptr_for_runtime(offset, data_count), data, data_count);
}

struct indirect_table_entry indirect_table[INDIRECT_TABLE_SIZE];

void add_function_to_table(u32 idx, u32 type_id, char* pointer) {
    awsm_assert(idx < INDIRECT_TABLE_SIZE);
    indirect_table[idx] = (struct indirect_table_entry){ .type_id = type_id, .func_pointer = pointer };
}

void clear_table() {
    for (int i = 0; i < INDIRECT_TABLE_SIZE; i++) {
        indirect_table[i] = (struct indirect_table_entry){ 0 };
    }
}

// The below functions are for implementing WASM instructions

// ROTL and ROTR helper functions
INLINE u32 rotl_u32(u32 n, u32 c_u32) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int       c    = c_u32 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT * sizeof(n) - 1); // assumes width is a power of 2.

    c &= mask;
    return (n << c) | (n >> ((-c) & mask));
}

INLINE u32 rotr_u32(u32 n, u32 c_u32) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int       c    = c_u32 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);

    c &= mask;
    return (n >> c) | (n << ((-c) & mask));
}

INLINE u64 rotl_u64(u64 n, u64 c_u64) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int       c    = c_u64 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT * sizeof(n) - 1); // assumes width is a power of 2.

    c &= mask;
    return (n << c) | (n >> ((-c) & mask));
}

INLINE u64 rotr_u64(u64 n, u64 c_u64) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int       c    = c_u64 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);

    c &= mask;
    return (n >> c) | (n << ((-c) & mask));
}

// Now safe division and remainder
INLINE u32 u32_div(u32 a, u32 b) {
    awsm_assert(b);
    return a / b;
}

INLINE u32 u32_rem(u32 a, u32 b) {
    awsm_assert(b);
    return a % b;
}

INLINE i32 i32_div(i32 a, i32 b) {
    awsm_assert(b && (a != INT32_MIN || b != -1));
    return a / b;
}

INLINE i32 i32_rem(i32 a, i32 b) {
    awsm_assert(b != 0);

    /* Because MIN is one less than MAX, we can FPE here */
    if (unlikely(a == INT32_MIN)) {
        return (a + abs(b)) % b;
    }

    return a % b;
}

INLINE u64 u64_div(u64 a, u64 b) {
    awsm_assert(b);
    return a / b;
}

INLINE u64 u64_rem(u64 a, u64 b) {
    awsm_assert(b);
    return a % b;
}

INLINE i64 i64_div(i64 a, i64 b) {
    awsm_assert(b && (a != INT64_MIN || b != -1));
    return a / b;
}

INLINE i64 i64_rem(i64 a, i64 b) {
    awsm_assert(b != 0);

    /* Because MIN is one less than MAX, we can FPE here */
    if (unlikely(a == INT64_MIN)) {
        return (a + labs(b)) % b;
    }

    return a % b;
}

// float to integer conversion methods
// In C, float => int conversions always truncate
// If a int2float(int::min_value) <= float <= int2float(int::max_value), it must always be safe to truncate it
u32 u32_trunc_f32(float f) {
    float integer_part = 0;
    float decimal_part = modff(f, &integer_part);

    if (unlikely(integer_part < 0 || integer_part > (float)UINT32_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (u32)f;
}

i32 i32_trunc_f32(float f) {
    float integer_part = 0;
    float decimal_part = modff(f, &integer_part);

    if (unlikely(integer_part < (float)INT32_MIN || integer_part > (float)INT32_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (i32)f;
}

u32 u32_trunc_f64(double f) {
    double integer_part = 0;
    double decimal_part = modf(f, &integer_part);

    if (unlikely(integer_part < 0 || integer_part > (double)UINT32_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }
    return (u32)f;
}

i32 i32_trunc_f64(double f) {
    double integer_part = 0;
    double decimal_part = modf(f, &integer_part);

    if (unlikely(integer_part < (double)INT32_MIN || integer_part > (double)INT32_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (i32)f;
}

u64 u64_trunc_f32(float f) {
    float integer_part = 0;
    float decimal_part = modff(f, &integer_part);

    if (unlikely(integer_part < 0 || integer_part > (float)UINT64_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (u64)f;
}

i64 i64_trunc_f32(float f) {
    float integer_part = 0;
    float decimal_part = modff(f, &integer_part);

    if (unlikely(integer_part < (float)INT64_MIN || integer_part > (float)INT64_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (i64)f;
}

u64 u64_trunc_f64(double f) {
    double integer_part = 0;
    double decimal_part = modf(f, &integer_part);

    if (unlikely(integer_part < 0 || integer_part > (double)UINT64_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (u64)f;
}

i64 i64_trunc_f64(double f) {
    double integer_part = 0;
    double decimal_part = modf(f, &integer_part);

    if (unlikely(integer_part < (double)INT64_MIN || integer_part > (double)INT64_MAX)) {
        fprintf(stderr, "integer overflow\n");
        awsm_assert(0);
    }

    return (i64)f;
}

// Float => Float truncation functions
INLINE float f32_trunc_f32(float f) {
    return truncf(f);
}

INLINE float f32_min(float a, float b) {
    return a < b ? a : b;
}

INLINE float f32_max(float a, float b) {
    return a > b ? a : b;
}

INLINE float f32_floor(float a) {
    return floorf(a);
}

INLINE float f32_ceil(float a) {
    return ceilf(a);
}

INLINE float f32_nearest(float a) {
    return nearbyintf(a);
}

INLINE float f32_copysign(float a, float b) {
    return copysignf(a, b);
}

INLINE double f64_trunc_f64(double f) {
    return trunc(f);
}

INLINE double f64_min(double a, double b) {
    return a < b ? a : b;
}

INLINE double f64_max(double a, double b) {
    return a > b ? a : b;
}

INLINE double f64_floor(double a) {
    return floor(a);
}

INLINE double f64_ceil(double a) {
    return ceil(a);
}

INLINE double f64_nearest(double a) {
    return nearbyint(a);
}

INLINE double f64_copysign(double a, double b) {
    return copysign(a, b);
}

__attribute__((noreturn)) void awsm_abi__trap_unreachable() {
    fprintf(stderr, "WebAssembly control flow unexpectedly reached unreachable instruction\n");
    exit(EXIT_FAILURE);
}

// We want to have some allocation logic here, so we can use it to implement libc
WEAK u32 wasmg___heap_base = 0;
u32      runtime_heap_base;

u32 allocate_n_bytes(u32 n) {
    u32 res = runtime_heap_base;
    runtime_heap_base += n;
    while (memory_size < runtime_heap_base) {
        expand_memory();
    }
    printf("rhb %d\n", runtime_heap_base);
    return res;
}

void* allocate_n_bytes_ptr(u32 n) {
    u32 addr = allocate_n_bytes(n);
    return get_memory_ptr_for_runtime(addr, n);
}

// If we are using runtime globals, we need to populate them
WEAK void populate_globals() {}

void runtime_init() {
    alloc_linear_memory();
    populate_table();
    switch_out_of_runtime();
    populate_globals();
    switch_into_runtime();
    populate_memory();

    // Rounding always is round-to-nearest ties-to-even, in correspondence with IEEE 754-2019
    // https://webassembly.github.io/spec/core/exec/numerics.html#floating-point-operations
    int rc = fesetround(FE_TONEAREST);
    awsm_assert(rc == 0);

    runtime_heap_base = wasmg___heap_base;
    if (runtime_heap_base == 0) {
        runtime_heap_base = memory_size;
    }
}
