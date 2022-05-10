#include "../../runtime/runtime.h"

IMPORT int wasmf_increment(void);

int main(int argc, char* argv[]) {
    runtime_init();

    awsm_assert(wasmf_increment() == 43);
    awsm_assert(wasmf_increment() == 44);
}
