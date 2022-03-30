#include "../../runtime/runtime.h"

/* Collatz Conjecture
 * Description at https://exercism.org/tracks/wasm/exercises/collatz-conjecture
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/collatz-conjecture/collatz-conjecture.spec.js
 */

IMPORT int wasmf_steps(int number);

int main(int argc, char* argv[]) {
    runtime_init();

    assert(wasmf_steps(1) == 0);
    assert(wasmf_steps(16) == 4);
    assert(wasmf_steps(12) == 9);
    assert(wasmf_steps(1000000) == 152);

    // number <= 0 triggers error
    assert(wasmf_steps(0) == -1);
    assert(wasmf_steps(-15) == -1);
}
