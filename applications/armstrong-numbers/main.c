#include <stdbool.h>

#include "../../runtime/runtime.h"

IMPORT bool wasmf_isArmstrongNumber(int);

int main(int argc, char* argv[]) {
    runtime_init();

    awsm_assert(wasmf_isArmstrongNumber(0) == true);
    awsm_assert(wasmf_isArmstrongNumber(5) == true);
    awsm_assert(wasmf_isArmstrongNumber(10) == false);
    awsm_assert(wasmf_isArmstrongNumber(153) == true);
    awsm_assert(wasmf_isArmstrongNumber(100) == false);
    awsm_assert(wasmf_isArmstrongNumber(9474) == true);
    awsm_assert(wasmf_isArmstrongNumber(9475) == false);
    awsm_assert(wasmf_isArmstrongNumber(9926315) == true);
    awsm_assert(wasmf_isArmstrongNumber(9926314) == false);
}
