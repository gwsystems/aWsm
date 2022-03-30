#include "../../runtime/runtime.h"

IMPORT int wasmf_open(void);
IMPORT int wasmf_close(void);
IMPORT int wasmf_deposit(int);
IMPORT int wasmf_withdraw(int);
IMPORT int wasmf_balance(void);

int main(int argc, char* argv[]) {
    runtime_init();

    // Actions return status code -1 when account closed
    assert(wasmf_balance() == -1);
    assert(wasmf_withdraw(100) == -1);
    assert(wasmf_deposit(100) == -1);
    assert(wasmf_close() == -1);

    // Should open succesfully
    assert(wasmf_open() == 0);

    // Actions return -2 if open but values invalid or withdraws exceed balance
    assert(wasmf_withdraw(100) == -2);
    assert(wasmf_deposit(-100) == -2);

    // Balance still 0 becauce deposit and withdraw failed
    assert(wasmf_balance() == 0);

    // Successful deposit and withdraw
    assert(wasmf_deposit(100) == 0);
    assert(wasmf_withdraw(50) == 0);
    assert(wasmf_balance() == 50);

    // Successful close
    assert(wasmf_close() == 0);
}
