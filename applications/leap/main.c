#include <stdbool.h>

#include "../../runtime/runtime.h"

/* leap
 * Description at https://exercism.org/tracks/wasm/exercises/leap
 * Asserts based on
 * https://github.com/exercism/wasm/blob/main/exercises/practice/leap/leap.spec.js
 */

IMPORT bool wasmf_isLeap(int year);

int main(int argc, char* argv[]) {
    runtime_init();

    assert(!wasmf_isLeap(2015));
    assert(!wasmf_isLeap(1970));
    assert(wasmf_isLeap(1996));
    assert(wasmf_isLeap(1960));
    assert(!wasmf_isLeap(2100));
    assert(!wasmf_isLeap(1900));
    assert(wasmf_isLeap(2000));
    assert(wasmf_isLeap(2400));
    assert(!wasmf_isLeap(1800));
}
