#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <wasi/api.h>

int main(int argc, char **argv)
{
    const size_t randomness_len = 10;
    uint8_t randomness[randomness_len] = {0};

    printf("Pre\n");
    for (size_t i = 0; i < randomness_len; i++)
        printf("%u,", randomness[i]);
    printf("\n");

    __wasi_errno_t rc = __wasi_random_get(randomness, randomness_len);
    assert(rc == __WASI_ERRNO_SUCCESS);

    printf("Post\n");
    for (size_t i = 0; i < randomness_len; i++)
        printf("%u,", randomness[i]);
    printf("\n");

    exit(EXIT_SUCCESS);
}
