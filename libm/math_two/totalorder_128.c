/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#define _ISOC23_SOURCE
#define WANT_FLOAT128

#include "math_two.h"

#ifdef float_t

#ifdef MAG
#define NAME totalordermag
#define NORMALIZE(h, l)            \
    do {                           \
        h &= 0x7fffffffffffffffLL; \
    } while (0)
#else
#define NAME totalorder
#define NORMALIZE(h, l)          \
    do {                         \
        uint64_t sign = h >> 63; \
        h ^= sign >> 1;          \
        l ^= sign;               \
    } while (0)
#endif

int
name(NAME)(const float_t *x, const float_t *y)
{
    int64_t  hx;
    uint64_t lx;
    GET_LDOUBLE_WORDS64(hx, lx, *x);

    int64_t  hy;
    uint64_t ly;
    GET_LDOUBLE_WORDS64(hy, ly, *y);

    if (!_IEEE_754_2008_SNAN) {
        if (IS_NAN(GET_ABS(hx), lx))
            hx ^= FLOAT_SNAN_BIT;
        if (IS_NAN(GET_ABS(hy), ly))
            hy ^= FLOAT_SNAN_BIT;
    }

    NORMALIZE(hx, lx);
    NORMALIZE(hy, ly);

    if (hy == hx)
        return lx <= ly;
    return hx <= hy;
}

#endif
