#include <string.h>

#include "../../runtime/runtime.h"

/* binary-search
 * Description at https://exercism.org/tracks/wasm/exercises/binary-search
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/binary-search/binary-search.spec.js
 */

const uint32_t bufferOffset   = 64;
const uint32_t bufferCapacity = 256; /* in elems */
const uint32_t elemSize       = 4;

#define FIRST_LEN 1
int first[FIRST_LEN] = { 6 };
#define SECOND_LEN 7
int second[SECOND_LEN] = { 1, 3, 4, 6, 8, 9, 11 };
#define THIRD_LEN 13
int third[THIRD_LEN] = { 1, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 634 };
#define FOURTH_LEN 12
int fourth[FOURTH_LEN] = { 1, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377 };
#define FIFTH_LEN 7
int fifth[FIFTH_LEN] = { 1, 3, 4, 6, 8, 9, 11 };
#define SIXTH_LEN 0
int sixth[SIXTH_LEN] = {};
#define SEVENTH_LEN 7
int seventh[SEVENTH_LEN] = { 1, 3, 4, 6, 8, 9, 11 };

IMPORT int32_t wasmf_find(uint32_t haystack_offset, uint32_t haystack_length, int32_t needle);

/**
 * Searches for a needle in a haystack of int32_t
 * "glue code" that copies a string into a buffer within the linear memory and then invokes the wasm export function.
 */
uint32_t find(int* haystack, uint32_t haystackLength, int32_t needle) {
    awsm_assert(haystackLength <= bufferCapacity);

    char* buffer = get_memory_ptr_void(bufferOffset, bufferCapacity);
    memmove(buffer, haystack, haystackLength * sizeof(needle));

    return wasmf_find(bufferOffset, haystackLength, needle);
}

int main(int argc, char* argv[]) {
    runtime_init();

    awsm_assert(find(first, FIRST_LEN, 6) == 0);
    awsm_assert(find(second, SECOND_LEN, 6) == 3);
    awsm_assert(find(second, SECOND_LEN, 1) == 0);
    awsm_assert(find(second, SECOND_LEN, 11) == 6);
    awsm_assert(find(third, THIRD_LEN, 144) == 9);
    awsm_assert(find(fourth, FOURTH_LEN, 21) == 5);
    awsm_assert(find(fifth, FIFTH_LEN, 0) == -1);
    awsm_assert(find(fifth, FIFTH_LEN, 13) == -1);
    awsm_assert(find(sixth, SIXTH_LEN, 1) == -1);
    awsm_assert(find(seventh, SEVENTH_LEN, 0) == -1);
}
