#include "runtime.h"

// Region initialization helper function
EXPORT void initialize_region(u32 offset, u32 data_count, char* data) {
    assert(memory_size >= data_count);
    assert(offset < memory_size - data_count);

    // FIXME: Hack around segmented and unsegmented access
    memcpy(get_memory_ptr_for_runtime(offset, data_count), data, data_count);
}

struct indirect_table_entry indirect_table[INDIRECT_TABLE_SIZE];

void add_function_to_table(u32 idx, u32 type_id, char* pointer) {
    assert(idx < INDIRECT_TABLE_SIZE);
    indirect_table[idx] = (struct indirect_table_entry) { .type_id = type_id, .func_pointer = pointer };
}

// ROTL and ROTR helper functions
INLINE u32 rotl_u32(u32 n, u32 c_u32) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int c = c_u32 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);  // assumes width is a power of 2.

    c &= mask;
    return (n<<c) | (n>>( (-c)&mask ));
}

INLINE u32 rotr_u32(u32 n, u32 c_u32) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int c = c_u32 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);

    c &= mask;
    return (n>>c) | (n<<( (-c)&mask ));
}

INLINE u64 rotl_u64(u64 n, u64 c_u64) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int c = c_u64 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);  // assumes width is a power of 2.

    c &= mask;
    return (n<<c) | (n>>( (-c)&mask ));
}

INLINE u64 rotr_u64(u64 n, u64 c_u64) {
    // WASM requires a modulus here (usually a single bitwise op, but it means we need no assert)
    unsigned int c = c_u64 % (CHAR_BIT * sizeof(n));
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);

    c &= mask;
    return (n>>c) | (n<<( (-c)&mask ));
}


// float to integer conversion methods
// In C, float => int conversions always truncate
// If a int2float(int::min_value) <= float <= int2float(int::max_value), it must always be safe to truncate it
u32 u32_trunc_f32(float f) {
    assert(0 <= f && f <= UINT32_MAX);
    return (u32) f;
}

i32 i32_trunc_f32(float f) {
    assert(INT32_MIN <= f && f <= INT32_MAX );
    return (i32) f;
}

u32 u32_trunc_f64(double f) {
    assert(0 <= f && f <= UINT32_MAX);
    return (u32) f;
}

i32 i32_trunc_f64(double f) {
    assert(INT32_MIN <= f && f <= INT32_MAX );
    return (i32) f;
}

u64 u64_trunc_f32(float f) {
    assert(0 <= f && f <= UINT64_MAX);
    return (u64) f;
}

i64 i64_trunc_f32(float f) {
    assert(INT64_MIN <= f && f <= INT64_MAX);
    return (i64) f;
}

u64 u64_trunc_f64(double f) {
    assert(0 <= f && f <= UINT64_MAX);
    return (u64) f;
}

i64 i64_trunc_f64(double f) {
    assert(INT64_MIN <= f && f <= INT64_MAX );
    return (i64) f;
}

// Float => Float truncation functions
INLINE float f32_trunc_f32(float f) {
    return trunc(f);
}

INLINE double f64_trunc_f64(double f) {
    return trunc(f);
}

// If we are using runtime globals, we need to populate them
WEAK void populate_globals() {}

// Code that actually runs the wasm code
IMPORT i32 wasmf_main(i32 a, i32 b);

int main(int argc, char* argv[]) {
    // Setup the linear memory and function table
    alloc_linear_memory();
    populate_table();

    switch_out_of_runtime();
    populate_globals();
    switch_into_runtime();
    populate_memory();

    // What follows is a huge cludge
    // We want to pass the program arguments to the program, so we need to copy them into an array in linear memory
    // This is about as nice as you would expect...

    u32 page_offset = memory_size;

    switch_out_of_runtime();
    expand_memory();
    switch_into_runtime();

    // FIXME: Should do a real bounds check here
    i32* array_ptr = get_memory_ptr_void(page_offset, argc * sizeof(i32));
    int string_offset = page_offset + argc * sizeof(i32);
    for (int i = 0; i < argc; i++) {
        size_t str_size = strlen(argv[i]) + 1;

        switch_out_of_runtime();
        array_ptr[i] = string_offset;
        switch_into_runtime();

        strcpy(get_memory_ptr_for_runtime(string_offset, strlen(argv[i]) + 1), argv[i]);

        string_offset += str_size;
    }

    stub_init(string_offset);

    switch_out_of_runtime();
    int ret =  wasmf_main(argc, page_offset);
    switch_into_runtime();
    return ret;
}
