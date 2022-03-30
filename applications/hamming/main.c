#include <string.h>

#include "../../runtime/runtime.h"

/* hamming
 * Description at https://exercism.org/tracks/wasm/exercises/hamming
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/hamming/hamming.spec.js
 */

const uint32_t firstBufferOffset   = 1024;
const uint32_t firstBufferCapacity = 1024;
uint32_t       firstBufferLength   = 0;

const uint32_t secondBufferOffset   = 2048;
const uint32_t secondBufferCapacity = 1024;
uint32_t       secondBufferLength   = 0;

IMPORT uint32_t wasmf_compute(uint32_t firstOffset, uint32_t firstLength, uint32_t secondOffset, uint32_t secondLength);

/**
 * Computes the Hamming distance of two strings
 * "glue code" that copies a string into a buffer within the linear memory and then invokes the wasm export function.
 */
uint32_t compute(char* first, char* second) {
    char* firstBuffer  = get_memory_ptr_void(firstBufferOffset, firstBufferCapacity);
    char* secondBuffer = get_memory_ptr_void(secondBufferOffset, secondBufferCapacity);

    strncpy(firstBuffer, first, firstBufferCapacity);
    firstBufferLength = strnlen(firstBuffer, firstBufferCapacity);

    strncpy(secondBuffer, second, secondBufferCapacity);
    secondBufferLength = strnlen(secondBuffer, secondBufferCapacity);

    return wasmf_compute(firstBufferOffset, firstBufferLength, secondBufferOffset, secondBufferLength);
}

int main(int argc, char* argv[]) {
    runtime_init();

    assert(compute("A", "A") == 0);
    assert(compute("G", "T") == 1);

    assert(compute("GGACTGAAATCTG", "GGACTGAAATCTG") == 0);
    assert(compute("GGACGGATTCTG", "AGGACGGATTCT") == 9);
    assert(compute("AATG", "AAA") == -1);
    assert(compute("ATA", "AGTG") == -1);
    assert(compute("", "G") == -1);
    assert(compute("G", "") == -1);
}
