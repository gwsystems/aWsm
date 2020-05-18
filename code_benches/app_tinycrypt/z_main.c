#include <stdio.h>

int aes_main(void);
int cbc_mode_main(void);
int ccm_mode_main(void);
int cmac_mode_main(void);
int ctr_mode_main(void);
int ctr_prng_main(void);
int ecc_dh_main(void);
int ecc_dsa_main(void);
int hmac_main(void);
int hmac_prng_main(void);
int sha256_main(void);

extern int _compress_count;

int main(void) {
// Blow the stack
//     ecc_dh_main();
//     ecc_dsa_main();

// Alighment issues
//    ccm_mode_main();

// Blow the heap
//    ctr_prng_main();

    aes_main();
    cbc_mode_main();
    cmac_mode_main();
    ctr_mode_main();
    hmac_main();
    hmac_prng_main();
    sha256_main();

//    printf("%d\n", _compress_count);

    return 0;
}
