#include "../../runtime/runtime.h"

IMPORT void wasmf_run(void);

int main(int argc, char* argv[]) {
    runtime_init();
    wasmf_run();
    return 0;
}
