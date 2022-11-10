/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdio_private.h"
#include <float.h>

#if __LDBL_MAX_10_EXP__ >= 4096
#define NPOW_10	13
#elif __LDBL_MAX_10_EXP__ >= 2048
#define NPOW_10	12
#elif __LDBL_MAX_10_EXP__ >= 1024
#define NPOW_10 11
#elif __LDBL_MAX_10_EXP__ >= 512
#define NPOW_10 10
#elif __LDBL_MAX_10_EXP__ >= 256
#define NPOW_10 9
#elif __LDBL_MAX_10_EXP__ >= 128
#define NPOW_10 8
#elif __LDBL_MAX_10_EXP__ >= 64
#define NPOW_10 7
#elif __LDBL_MAX_10_EXP__ >= 32
#define NPOW_10 6
#else
#error __LDBL_MAX_10_EXP__ too small
#endif

static const long double pwr_p10 [NPOW_10] = {
    1e+1L, 1e+2L, 1e+4L, 1e+8L, 1e+16L, 1e+32L,
#if NPOW_10 >= 7
    1e+64L,
#endif
#if NPOW_10 >= 8
    1e+128L,
    #endif
#if NPOW_10 >= 9
    1e+256L,
#endif
#if NPOW_10 >= 10
    1e+512L,
#endif
#if NPOW_10 >= 11
    1e+1024L,
#endif
#if NPOW_10 >= 12
    1e+2048L,
#endif
#if NPOW_10 >= 13
    1e+4096L
#endif
};

static const long double pwr_m10 [NPOW_10] = {
    1e-1L, 1e-2L, 1e-4L, 1e-8L, 1e-16L, 1e-32L,
#if NPOW_10 >= 7
    1e-64L,
#endif
#if NPOW_10 >= 8
    1e-128L,
#endif
#if NPOW_10 >= 9
    1e-256L,
#endif
#if NPOW_10 >= 10
    1e-512L,
#endif
#if NPOW_10 >= 11
    1e-1024L,
#endif
#if NPOW_10 >= 12
    1e-2048L,
#endif
#if NPOW_10 >= 13
    1e-4096L
#endif
};

long double
__atold_engine(_u128 m10, int e10)
{
    long double flt = _u128_to_ld(m10);
    int pwr;
    const long double *pptr;

    if (e10 < 0) {
        pptr = (pwr_m10 + NPOW_10 - 1);
        e10 = -e10;
    } else {
        pptr = (pwr_p10 + NPOW_10 - 1);
    }
    for (pwr = 1 << (NPOW_10 - 1); pwr; pwr >>= 1) {
        for (; e10 >= pwr; e10 -= pwr) {
            flt *= *pptr;
        }
        pptr--;
    }
    return flt;
}
