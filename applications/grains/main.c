#include "../../runtime/runtime.h"

/* Grains
 * Description at https://exercism.org/tracks/wasm/exercises/grains
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/grains/grains.spec.js
 */

IMPORT uint64_t wasmf_square(int squareNum);
IMPORT uint64_t wasmf_total(void);

int main(int argc, char* argv[]) {
    runtime_init();

    awsm_assert(wasmf_square(1) == 1UL);
    awsm_assert(wasmf_square(2) == 2UL);
    awsm_assert(wasmf_square(3) == 4UL);
    awsm_assert(wasmf_square(4) == 8UL);
    awsm_assert(wasmf_square(16) == 32768UL);
    awsm_assert(wasmf_square(32) == 2147483648UL);
    awsm_assert(wasmf_square(64) == 9223372036854775808UL);

    awsm_assert(wasmf_total() == 18446744073709551615UL);
}
