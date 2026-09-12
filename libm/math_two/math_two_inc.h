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

#define __name(a, b)   a##b
#define _name(a, b)    __name(a, b)
#define name(a)        _name(a, NAME_SUFFIX)
#define kernel(a)      _name(a, KERNEL_SUFFIX)

#define fma_f(a, b, c) name(fma)(a, b, c)
#define floor_f(x)     name(floor)(x)
#define frexp_f(x, a)  name(frexp)(x, a)
#define ldexp_f(x, a)  name(ldexp)(x, a)

#define ff_t           name(ff)

#ifdef fuint_t
static inline fuint_t
name(touint)(float_t x)
{
    union {
        float_t f;
        fuint_t u;
    } u;
    u.f = x;
    return u.u;
}

static inline float_t
name(tofloat)(fuint_t x)
{
    union {
        float_t f;
        fuint_t u;
    } u;
    u.u = x;
    return u.f;
}
#endif

typedef struct {
    float_t hi, lo;
} ff_t;

static inline ff_t
f_to_ff(float_t x)
{
    return (ff_t) { .hi = x, .lo = 0 };
}

/* From https://hal.archives-ouvertes.fr/hal-01351529v3/document */

#ifndef __HAVE_FAST_FMA_F
#define SPLIT   (float_f(SPLIT_VAL) + float_f(1.0))
#define split_f name(split)

static inline ff_t
split_f(float_t x)
{
    float_t a = SPLIT * x;
    float_t b = x - a;
    float_t hi = a + b;
    float_t lo = x - hi;
    return (ff_t) { .hi = hi, .lo = lo };
}
#endif

#define f_add_f_fast name(f_add_f_fast_)

static inline ff_t
f_add_f_fast(float_t x, float_t y)
{
    float_t s, z, t;
    s = x + y;
    z = s - x;
    t = y - z;
    return (ff_t) { .hi = s, .lo = t };
}

#define f_add_f name(f_add_f_)

static inline ff_t
f_add_f(float_t a, float_t b)
{
    float_t s, ap, bp, da, db, t;

    s = a + b;
    ap = s - b;
    bp = s - ap;
    da = a - ap;
    db = b - bp;
    t = da + db;
    return (ff_t) { .hi = s, .lo = t };
}

#define ff_add_f name(ff_add_f_)

static inline ff_t
ff_add_f(ff_t x, float_t y)
{
    ff_t    s = f_add_f(x.hi, y);
    float_t v = x.lo + s.lo;
    return f_add_f_fast(s.hi, v);
}

#define ff_add_ff name(ff_add_ff_)

static inline ff_t
ff_add_ff(ff_t x, ff_t y)
{
    ff_t    s = f_add_f(x.hi, y.hi);
    ff_t    t = f_add_f(x.lo, y.lo);
    float_t c = s.lo + t.hi;
    ff_t    v = f_add_f_fast(s.hi, c);
    float_t w = t.lo + v.lo;
    return f_add_f_fast(v.hi, w);
}

#define f_mul_f name(f_mul_f_)

static inline ff_t
f_mul_f(float_t x, float_t y)
{
#ifdef __HAVE_FAST_FMA_F
    /* 2mul with FMA */
    float_t hi = x * y;
    float_t lo = fma_f(x, y, -hi);
    return (ff_t) {
        .hi = hi,
        .lo = lo,
    };
#else
    /* 2mul using Dekker's algorithm */
    int     xexp, yexp;
    float_t xi = frexp_f(x, &xexp);
    float_t yi = frexp_f(y, &yexp);
    ff_t    xs = split_f(xi);
    ff_t    ys = split_f(yi);
    float_t r1 = xi * yi;
    float_t t1 = xs.hi * ys.hi - r1;
    float_t t2 = xs.hi * ys.lo + t1;
    float_t t3 = xs.lo * ys.hi + t2;
    float_t r2 = xs.lo * ys.lo + t3;
    float_t hi = ldexp_f(r1, xexp + yexp);
    float_t lo = ldexp_f(r2, xexp + yexp);
    if (!isfinite(hi))
        lo = 0;
    return (ff_t) { .hi = hi, .lo = lo };
#endif
}

#define ff_mul_f name(ff_mul_f_)

static inline ff_t
ff_mul_f(ff_t x, float_t y)
{
#ifdef __HAVE_FAST_FMA_F
    ff_t    c = f_mul_f(x.hi, y);
    float_t cl3 = fma_f(x.lo, y, c.lo);
    return f_add_f_fast(c.hi, cl3);
#else
    ff_t    c = f_mul_f(x.hi, y);
    float_t cl2 = x.lo * y;
    ff_t    t = f_add_f_fast(c.hi, cl2);
    float_t tl2 = t.lo + c.lo;
    return f_add_f_fast(t.hi, tl2);
#endif
}

#define ff_mul_ff name(ff_mul_ff_)

static inline ff_t
ff_mul_ff(ff_t x, ff_t y)
{
#ifdef __HAVE_FAST_FMA_F
    ff_t c = f_mul_f(x.hi, y.hi);
    if (isinf(c.hi))
        return c;
    float_t tl = x.hi * y.lo;
    float_t cl2 = fma_f(x.lo, y.hi, tl);
    float_t cl3 = c.lo + cl2;
    return f_add_f_fast(c.hi, cl3);
#else
    ff_t c = f_mul_f(x.hi, y.hi);
    if (isinf(c.hi))
        return c;
    float_t tl1 = x.hi * y.lo;
    float_t tl2 = x.lo * y.hi;
    float_t cl2 = tl1 + tl2;
    float_t cl3 = c.lo + cl2;
    return f_add_f_fast(c.hi, cl3);
#endif
}

#define ff_div_f name(ff_div_f_)

static inline ff_t
ff_div_f(ff_t x, float_t y)
{
    float_t th = x.hi / y;
    ff_t    pi = f_mul_f(th, y);
    ff_t    de1 = f_add_f(x.hi, -pi.hi);
    float_t del2 = x.lo - pi.lo;
    float_t del = de1.lo + del2;
    float_t de = de1.hi + del;
    float_t tl = de / y;
    return f_add_f_fast(th, tl);
}

#define ff_div_ff name(ff_div_ff_)

static inline ff_t
ff_div_ff(ff_t x, ff_t y)
{
#ifdef __HAVE_FAST_FMA_F
    /* Algorithm 18 */
    if (isinf(y.hi))
        return (ff_t) { .hi = 0, .lo = 0 };
    if (y.hi == 0)
        return (ff_t) { .hi = x.hi / y.hi, .lo = 0 };
    float_t th = 1 / y.hi;
    if (isinf(th))
        return (ff_t) { .hi = th, .lo = 0 };
    float_t rh = -fma_f(y.hi, th, -1);
    float_t rl = -y.lo * th;
    ff_t    e = f_add_f_fast(rh, rl);
    ff_t    de = ff_mul_f(e, th);
    ff_t    m = ff_add_f(de, th);
    return ff_mul_ff(x, m);
#else
    /* Algorithm 17 */
    if (isinf(y.hi))
        return (ff_t) { .hi = 0, .lo = 0 };
    if (y.hi == 0)
        return (ff_t) { .hi = 1 / y.hi, .lo = 0 };
    float_t th = x.hi / y.hi;
    if (isinf(th))
        return (ff_t) { .hi = th, .lo = 0 };
    ff_t    r = ff_mul_f(y, th);
    float_t pih = x.hi - r.hi;
    float_t del = x.lo - r.lo;
    float_t de = pih + del;
    float_t tl = de / y.hi;
    return f_add_f_fast(th, tl);
#endif
}

float_t           name(_ff_scalbn)(ff_t x, int expo);
int               name(_rem_half)(float_t x, float_t *y);
float_t           name(_ff_pown)(float_t hi, float_t lo, long long int k);

extern const ff_t name(_pi_ff_);
extern const ff_t name(_inv_pi_ff_);
