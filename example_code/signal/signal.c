#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* 
 * We have to manually define this because wasi-libc errors out if we include <signal.h>
 * complaining that WebAssembly requires various flags to emulate signals. This is because
 * WASI does not seem to have a means to support registering signal handlers. WASI does
 * provide a call to raise a signal, which gets delivered to the runtime process.
 */
#define SIGINT 2

int main(int argc, char **argv)
{
    /* 
     * Send SIGINT to runtime process
     * The string should not print, and the status code of the runtime should be
     * EOWNERDEAD 130 Owner died
     */
    uint32_t rc = __wasi_proc_raise(SIGINT);
    printf("Did I reach here?\n");
    exit(EXIT_SUCCESS);
}
