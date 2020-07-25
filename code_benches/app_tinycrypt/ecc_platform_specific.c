/*  uECC_platform_specific.c - Implementation of platform specific functions*/

/* Copyright (c) 2014, Kenneth MacKay
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.*/

/*
 *  Copyright (C) 2017 by Intel Corporation, All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *    - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *    - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    - Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  uECC_platform_specific.c -- Implementation of platform specific functions
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int _CSPRNG_counter = 0;
const uint8_t _CSPRNG_data[] = {225,203,132,106,100,16,26,196,13,68,227,166,163,16,162,203,34,119,7,157,194,130,228,100,211,64,234,197,112,54,174,138,201,80,161,155,91,166,215,128,201,91,121,146,10,220,34,71,248,141,230,250,83,197,193,46,110,111,79,180,9,90,243,34,210,133,58,80,30,164,31,99,19,88,108,185,91,17,209,55,255,182,6,108,116,130,140,227,111,32,54,119,107,211,158,49,232,72,4,215,197,60,54,75,177,89,26,200,166,133,29,251,26,26,181,56,126,41,148,219,186,192,166,50,159,229,95,101,141,126,192,23,198,92,54,206,213,138,38,55,210,129,28,9,97,13,251,147,123,27,6,242,6,200,173,155,29,91,80,177,72,152,191,41,125,105,70,60,83,60,248,56,184,156,249,34,22,6,99,191,230,27,252,93,45,201,220,230,236,238,12,86,191,74,78,8,218,63,71,142,33,195,28,239,133,192,179,121,144,189,39,24,62,166,45,53,83,128,61,93,30,3,39,208,230,80,124,203,139,156,85,215,179,158,176,160,178,151,251,185,241,70,75,212,32,143,92,45,131,158,32,198,238,63,0,39,116,106,105,75,68,58,110,90,27,122,9,44,65,35,239,3,166,54,128,252,216,146,125,3,190,70,107,220,140,255,53,225,250,66,165,8,193,22,146,124,251,156,218,251,1,193,107,36,39,166,227,115,79,35,207,251,207,246,97,20,136,58,110,79,51,41,97,232,253,32,31,88,82,63,74,181,180,84,88,207,93,177,191,70,76,123,236,243,145,58,20,157,210,71,75,96,43,185,63,44,248,166,224,211,232,147,42,231,16,201,79,22,39,16,135,246,238,2,220,78,1,172,134,62,73,83,216,131,19,186,0,165,45,225,130,84,133,35,123,241,255,6,159,232,7,196,144,40,26,25,2,195,122,25,173,120,81,40,69,140,157,205,201,38,207,249,246,202,6,241,251,248,107,79,180,66,140,237,188,165,169,114,123,62,65,62,205,229,15,185,62,221,156,219,36,61,145,64,146,149,40,93,124,142,113,161,224,142,98,74,117,85,137,189,145,208,206,191,252,112,214,105,179,184,32,236,192,37,29,208,48,26,63,86,93,161,208,70,149,250,143,52,94,85};
int default_CSPRNG(uint8_t *dest, unsigned int size) {
    for (int i = 0; i < size; i++) {
//        printf("rng counter %d\n", _CSPRNG_counter);

        dest[i] = _CSPRNG_data[_CSPRNG_counter];

        _CSPRNG_counter = (_CSPRNG_counter + 1) % (sizeof(_CSPRNG_data) / sizeof(_CSPRNG_data[0]));
    }
    return 1;
}

