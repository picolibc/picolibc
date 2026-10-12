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
#define WANT_FLOAT80

#include "math_two.h"

#ifdef float_t

#ifdef MAG
#define NAME totalordermag
#define NORMALIZE(es, h, l) \
    do {                    \
        es &= 0x7fff;       \
    } while (0)
#else
#define NAME totalorder
#define NORMALIZE(es, h, l)                          \
    do {                                             \
        uint32_t sign = es >> 31;                    \
        es ^= (sign >> 17);                          \
        h ^= sign;                                   \
        l ^= sign;                                   \
    } while (0)
#endif

int
name(NAME)(const float_t *x, const float_t *y)
{
    uint32_t hx, lx;
    int32_t  esx;
    GET_LDOUBLE_WORDS(esx, hx, lx, *x);
    NORMALIZE(esx, hx, lx);

    uint32_t hy, ly;
    int32_t  esy;
    GET_LDOUBLE_WORDS(esy, hy, ly, *y);
    NORMALIZE(esy, hy, ly);

    if (esy == esx) {
        if (hy == hx)
            return lx <= ly;
        return hx <= hy;
    }
    return esx <= esy;
}

#endif
