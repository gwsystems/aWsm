#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wasi/api.h>

int main(int argc, char **argv)
{
    /* Print UNIX time */
    time_t now;
    time(&now);
    printf("Seconds since UNIX epoch: %ld\n", now);

    /* Print Date */
    char buf[100];
    strftime(buf, sizeof(buf), "%A, %x\n", localtime(&now));
    printf("Today is %s", buf);

    /* Check Clock resolution */
    __wasi_timestamp_t ts;
    __wasi_errno_t rc;
    rc = __wasi_clock_res_get(__WASI_CLOCKID_REALTIME, &ts);
    assert(rc == __WASI_ERRNO_SUCCESS);
    printf("Realtime Resolution is %lu\n", ts);

    rc = __wasi_clock_res_get(__WASI_CLOCKID_MONOTONIC, &ts);
    assert(rc == __WASI_ERRNO_SUCCESS);
    printf("Monotonic Resolution is %lu\n", ts);

    rc = __wasi_clock_res_get(__WASI_CLOCKID_PROCESS_CPUTIME_ID, &ts);
    assert(rc == __WASI_ERRNO_SUCCESS);
    printf("Process CPU-time Clock Resolution is %lu\n", ts);

    rc = __wasi_clock_res_get(__WASI_CLOCKID_THREAD_CPUTIME_ID, &ts);
    assert(rc == __WASI_ERRNO_SUCCESS);
    printf("Thread CPU-time Clock Resolution is %lu\n", ts);

    exit(EXIT_SUCCESS);
}
