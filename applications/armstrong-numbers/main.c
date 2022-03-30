#include <stdbool.h>

#include "../../runtime/runtime.h"

IMPORT bool wasmf_isArmstrongNumber(int);

int main(int argc, char* argv[]) {
    runtime_init();

    assert(wasmf_isArmstrongNumber(0) == true);
    assert(wasmf_isArmstrongNumber(5) == true);
    assert(wasmf_isArmstrongNumber(10) == false);
    assert(wasmf_isArmstrongNumber(153) == true);
    assert(wasmf_isArmstrongNumber(100) == false);
    assert(wasmf_isArmstrongNumber(9474) == true);
    assert(wasmf_isArmstrongNumber(9475) == false);
    assert(wasmf_isArmstrongNumber(9926315) == true);
    assert(wasmf_isArmstrongNumber(9926314) == false);
}
