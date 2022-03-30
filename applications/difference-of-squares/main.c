#include "../../runtime/runtime.h"

/* Difference of Squares
 * Description at https://exercism.org/tracks/wasm/exercises/difference-of-squares
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/difference-of-squares/difference-of-squares.spec.js
 */

IMPORT int wasmf_squareOfSum(int max);
IMPORT int wasmf_sumOfSquares(int max);
IMPORT int wasmf_difference(int max);

int main(int argc, char* argv[]) {
    runtime_init();

    assert(wasmf_squareOfSum(1) == 1);
    assert(wasmf_squareOfSum(5) == 225);
    assert(wasmf_squareOfSum(100) == 25502500);

    assert(wasmf_sumOfSquares(1) == 1);
    assert(wasmf_sumOfSquares(5) == 55);
    assert(wasmf_sumOfSquares(100) == 338350);

    assert(wasmf_difference(1) == 0);
    assert(wasmf_difference(5) == 170);
    assert(wasmf_difference(100) == 25164150);
}
