#include "../../runtime/runtime.h"

/* Darts
 * Description at https://exercism.org/tracks/wasm/exercises/darts
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/darts/darts.spec.js
 */

IMPORT int wasmf_score(float x, float y);

int main(int argc, char* argv[]) {
    runtime_init();

    assert(wasmf_score(-9.0, -9.0) == 0);
    assert(wasmf_score(0.0, 10.0) == 1);
    assert(wasmf_score(-5.0, 0.0) == 5);
    assert(wasmf_score(0.0, -1.0) == 10);
    assert(wasmf_score(0.0, 0.0) == 10);
    assert(wasmf_score(-0.1, -0.1) == 10);
    assert(wasmf_score(0.7, 0.7) == 10);
    assert(wasmf_score(0.8, -0.8) == 5);
    assert(wasmf_score(-3.5, -3.5) == 5);
    assert(wasmf_score(-3.6, -3.6) == 1);
    assert(wasmf_score(-7.0, 7.0) == 1);
    assert(wasmf_score(7.1, -7.1) == 0);
    assert(wasmf_score(0.5, -4) == 5);
}
