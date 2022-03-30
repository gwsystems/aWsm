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

    assert(wasmf_square(1) == 1UL);
    assert(wasmf_square(2) == 2UL);
    assert(wasmf_square(3) == 4UL);
    assert(wasmf_square(4) == 8UL);
    assert(wasmf_square(16) == 32768UL);
    assert(wasmf_square(32) == 2147483648UL);
    assert(wasmf_square(64) == 9223372036854775808UL);

    assert(wasmf_total() == 18446744073709551615UL);
}
