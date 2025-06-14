/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#include <stdio.h>

#define __name(a,b) a ## b
#define _name(a,b) __name(a,b)
#define name(a) _name(a, NAME_SUFFIX)

#define fma_f(a,b,c)    name(fma)(a,b,c)
#define float_f(x)      name(float)(x)
#define floor_f(x)      name(floor)(x)
#define frexp_f(x,a)    name(frexp)(x,a)
#define ldexp_f(x,a)    name(ldexp)(x,a)

#define ff_t name(ff)

typedef struct {
    float_t     hi, lo;
} ff_t;

#ifndef HAVE_FAST_FMA_F

#define SPLIT   (float_f(SPLIT_VAL) + float_f(1.0))
#define split_f  name(split)

static inline ff_t
split_f(float_t x)
{
    float_t   a = SPLIT * x;
    float_t   b = x - a;
    float_t   hi = a + b;
    float_t   lo = x - hi;
    return (ff_t) { .hi = hi, .lo = lo };
}
#endif

#define two_mul name(two_mul)

static inline ff_t
two_mul(float_t x, float_t y)
{
#ifdef HAVE_FAST_FMA_F
    /* 2mul with FMA */
    float_t   hi = x * y;
    float_t   lo = fma_f(x, y, -hi);
    return (ff_t) { .hi = hi, .lo = lo, };
#else
    /* 2mul using Dekker's algorithm */
    int xexp, yexp;
    float_t     xi = frexp_f(x, &xexp);
    float_t     yi = frexp_f(y, &yexp);
    ff_t        xs = split_f(xi);
    ff_t        ys = split_f(yi);
    float_t     r1 = xi * yi;
    float_t     t1 = xs.hi * ys.hi - r1;
    float_t     t2 = xs.hi * ys.lo + t1;
    float_t     t3 = xs.lo * ys.hi + t2;
    float_t     r2 = xs.lo * ys.lo + t3;
    float_t     hi = ldexp_f(r1, xexp + yexp);
    float_t     lo = ldexp_f(r2, xexp + yexp);
    if (!isfinite(hi))
        lo = 0;
    return (ff_t) { .hi = hi, .lo = lo };
#endif
}
