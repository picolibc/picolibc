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

#ifndef _LOCAL_INT128_H_
#define _LOCAL_INT128_H_

/*
 * 128-bit unsigned integers and the operations picolibc uses on them,
 * with a struct of two 64-bit values where the compiler doesn't
 * provide __uint128_t.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __uint128_t _u128;
typedef __int128_t  _i128;
#define to_u128(x)             (x)
#define from_u128(x)           (x)
#define _u128_to_ld(a)         ((long double)(a))
#define _u128_is_zero(a)       ((a) == 0)
#define _i128_lt_zero(a)       ((_i128)(a) < 0)
#define _u128_plus_64(a, b)    ((a) + (b))
#define _u128_plus(a, b)       ((a) + (b))
#define _u128_minus(a, b)      ((a) - (b))
#define _u128_minus_64(a, b)   ((a) - (b))
#define _u128_times_10(a)      ((a) * 10)
#define _u128_times_base(a, b) ((a) * (b))
#define _u128_to_ld(a)         ((long double)(a))
#define _u128_oflow(a)                                                                   \
    ((a) >= (((((_u128)0xffffffffffffffffULL) << 64) | 0xffffffffffffffffULL) - 9 / 10))
#define _u128_zero            (_u128)0
#define _u128_lshift(a, b)    ((_u128)(a) << (b))
#define _u128_lshift_64(a, b) ((_u128)(a) << (b))
#define _u128_rshift(a, b)    ((a) >> (b))
#define _i128_rshift(a, b)    ((_i128)(a) >> (b))
#define _u128_or_64(a, b)     ((a) | (_u128)(b))
#define _u128_and_64(a, b)    ((uint64_t)(a) & (b))
#define _u128_or(a, b)        ((a) | (b))
#define _u128_and(a, b)       ((a) & (b))
#define _u128_eq(a, b)        ((a) == (b))
#define _u128_ge(a, b)        ((a) >= (b))
#define _i128_ge(a, b)        ((_i128)(a) >= (_i128)(b))
#define _u128_lt(a, b)        ((a) < (b))
#define _i128_lt(a, b)        ((_i128)(a) < (_i128)(b))
#define _u128_not(a)          (~(a))
#else
typedef struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t lo, hi;
#else
    uint64_t hi, lo;
#endif
} _u128;
#define _u128_zero (_u128) { 0, 0 }

static inline _u128
to_u128(uint64_t x)
{
    _u128 a = { .hi = 0, .lo = x };
    return a;
}

static inline uint64_t
from_u128(_u128 a)
{
    return a.lo;
}

static inline long double
_u128_to_ld(_u128 a)
{
    return (long double)a.hi * ((long double)(1LL << 32) * (long double)(1LL << 32))
        + (long double)a.lo;
}

static inline bool
_u128_is_zero(_u128 a)
{
    return a.hi == 0 && a.lo == 0;
}

static inline bool
_i128_lt_zero(_u128 a)
{
    return (int64_t)a.hi < 0;
}

static inline bool
_u128_eq(_u128 a, _u128 b)
{
    return (a.hi == b.hi) && (a.lo == b.lo);
}

static inline bool
_u128_lt(_u128 a, _u128 b)
{
    if (a.hi == b.hi)
        return a.lo < b.lo;
    return a.hi < b.hi;
}

static inline bool
_i128_lt(_u128 a, _u128 b)
{
    if (a.hi == b.hi) {
        if ((int64_t)a.hi < 0)
            return a.lo > b.lo;
        else
            return a.lo < b.lo;
    }
    return (int64_t)a.hi < (int64_t)b.hi;
}

static inline bool
_u128_ge(_u128 a, _u128 b)
{
    if (a.hi == b.hi)
        return a.lo >= b.lo;
    return a.hi >= b.hi;
}

static inline bool
_i128_ge(_u128 a, _u128 b)
{
    if (a.hi == b.hi) {
        if ((int64_t)a.hi < 0)
            return a.lo <= b.lo;
        else
            return a.lo >= b.lo;
    }
    return (int64_t)a.hi >= (int64_t)b.hi;
}

static inline _u128
_u128_plus_64(_u128 a, uint64_t b)
{
    _u128 v;

    v.lo = a.lo + b;
    v.hi = a.hi;
    if (v.lo < a.lo)
        v.hi++;
    return v;
}

static inline _u128
_u128_plus(_u128 a, _u128 b)
{
    _u128 v;

    v.lo = a.lo + b.lo;
    v.hi = a.hi + b.hi;
    if (v.lo < a.lo)
        v.hi++;
    return v;
}

static inline _u128
_u128_minus_64(_u128 a, uint64_t b)
{
    _u128 v;

    v.lo = a.lo - b;
    v.hi = a.hi;
    if (v.lo > a.lo)
        v.hi--;
    return v;
}

static inline _u128
_u128_minus(_u128 a, _u128 b)
{
    _u128 v;

    v.lo = a.lo - b.lo;
    v.hi = a.hi - b.hi;
    if (v.lo > a.lo)
        v.hi--;
    return v;
}

static inline _u128
_u128_lshift(_u128 a, int amt)
{
    _u128 v;

    if (amt == 0) {
        v = a;
    } else if (amt < 64) {
        v.lo = a.lo << amt;
        v.hi = (a.lo >> (64 - amt)) | (a.hi << amt);
    } else {
        v.lo = 0;
        v.hi = a.lo << (amt - 64);
    }
    return v;
}

static inline _u128
_u128_lshift_64(uint64_t a, int amt)
{
    _u128 v;

    if (amt == 0) {
        v.lo = a;
        v.hi = 0;
    } else if (amt < 64) {
        v.lo = a << amt;
        v.hi = (a >> (64 - amt));
    } else {
        v.lo = 0;
        v.hi = a << (amt - 64);
    }
    return v;
}

static inline _u128
_u128_rshift(_u128 a, int amt)
{
    _u128 v;

    if (amt == 0) {
        v = a;
    } else if (amt < 64) {
        v.lo = (a.hi << (64 - amt)) | (a.lo >> amt);
        v.hi = a.hi >> amt;
    } else {
        v.hi = 0;
        v.lo = a.hi >> (amt - 64);
    }
    return v;
}

static inline _u128
_u128_and(_u128 a, _u128 b)
{
    _u128 v;

    v.hi = a.hi & b.hi;
    v.lo = a.lo & b.lo;
    return v;
}

static inline uint64_t
_u128_and_64(_u128 a, uint64_t b)
{
    return a.lo & b;
}

static inline _u128
_u128_or(_u128 a, _u128 b)
{
    _u128 v;

    v.lo = a.lo | b.lo;
    v.hi = a.hi | b.hi;
    return v;
}

static inline _u128
_u128_or_64(_u128 a, uint64_t b)
{
    _u128 v;

    v.lo = a.lo | b;
    v.hi = a.hi;
    return v;
}

static inline _u128
_u128_not(_u128 a)
{
    _u128 v;

    v.lo = ~a.lo;
    v.hi = ~a.hi;
    return v;
}

static inline _u128
_u128_times_10(_u128 a)
{
    return _u128_plus(_u128_lshift(a, 3), _u128_lshift(a, 1));
}

static inline _u128
_u128_times_base(_u128 a, int base)
{
    if (base == 10)
        return _u128_times_10(a);
    return _u128_lshift(a, 4);
}

static inline bool
_u128_oflow(_u128 a)
{
    return a.hi >= (0xffffffffffffffffULL - 9) / 10;
}
#endif

static inline bool
_u128_gt(_u128 a, _u128 b)
{
    return _u128_lt(b, a);
}

#endif /* _LOCAL_INT128_H_ */
