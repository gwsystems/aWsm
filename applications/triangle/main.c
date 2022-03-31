#include "../../runtime/runtime.h"
#include <stdbool.h>

/* triangle
 * Description at https://exercism.org/tracks/wasm/exercises/triangle
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/triangle/triangle.spec.js
 */

IMPORT bool wasmf_isDegenate(float a, float b, float c);
IMPORT bool wasmf_isEquilateral(float a, float b, float c);
IMPORT bool wasmf_isIsosceles(float a, float b, float c);
IMPORT bool wasmf_isScalene(float a, float b, float c);

int main(int argc, char* argv[]) {
    runtime_init();

    assert(wasmf_isEquilateral(2, 2, 2));
    assert(!wasmf_isEquilateral(2, 3, 2));
    assert(!wasmf_isEquilateral(5, 4, 6));
    assert(wasmf_isEquilateral(0.5, 0.5, 0.5));
    assert(wasmf_isIsosceles(3, 4, 4));
    assert(wasmf_isIsosceles(4, 4, 3));
    assert(wasmf_isIsosceles(4, 4, 4));
    assert(!wasmf_isIsosceles(2, 3, 4));
    assert(!wasmf_isIsosceles(1, 1, 3));
    assert(!wasmf_isIsosceles(1, 3, 1));
    assert(!wasmf_isIsosceles(3, 1, 1));
    assert(wasmf_isIsosceles(0.5, 0.4, 0.5));
    assert(wasmf_isScalene(5, 4, 6));
    assert(!wasmf_isScalene(4, 4, 4));
    assert(!wasmf_isScalene(4, 4, 3));
    assert(!wasmf_isScalene(7, 3, 2));
    assert(wasmf_isScalene(0.5, 0.4, 0.6));
}
