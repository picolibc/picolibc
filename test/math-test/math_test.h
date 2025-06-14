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

#ifndef _MATH_TEST_H_
#define _MATH_TEST_H_
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

/* General float with up to 128 bits of significand */
typedef struct {
    enum { float_finite, float_inf, float_nan } kind;
    bool        neg;
    int32_t     exp;
    uint64_t    sig[2];
} float_gen_t;

#define fkind(f)        (isnan(f) ? float_nan : isinf(f) ? float_inf : float_finite)

#define _name(a,e)      a ## e

#if __FLT_MANT_DIG__ == 24
#define INCLUDE_FLOAT_32
typedef float float_32_t;
#define name_32(a)      _name(a,f)
#define format_32       "%a"
#define float_32(a)     a ## f
#endif

#if __DBL_MANT_DIG__ == 53
#define INCLUDE_FLOAT_64
typedef double float_64_t;
#define name_64(a)      _name(a,)
#define format_64       "%a"
#define float_64(a)     a
#elif __LDBL_MANT_DIG__ == 53
#define INCLUDE_FLOAT_64
typedef long double float_64_t;
#define name_64(a)      _name(a,l)
#define format_64       "%La"
#define float_64(a)     a ## l
#endif

#if __LDBL_MANT_DIG__ == 64
#define INCLUDE_FLOAT_80
typedef long double float_80_t;
#define name_80(a)      _name(a,l)
#define format_80       "%La"
#define float_80(a)     a ## l
#endif

#if __LDBL_MANT_DIG__ == 113
#define INCLUDE_FLOAT_128
typedef long double float_128_t;
#define name_128(a)     _name(a,l)
#define format_128       "%La"
#define float_128(a)    a ## l
#endif

typedef struct {
    uint32_t    exp_err;
    uint64_t    ulp;
} err_t;

#ifdef INCLUDE_FLOAT_32
typedef struct {
    float_32_t  x, y;
} vector_32;

static inline float_gen_t unpack_32(float_32_t f)
{
    int         exp;
    float_32_t  s = name_32(frexp)(f, &exp);
    (void) s;
    union {
        uint32_t        u;
        float_32_t      f;
    } u;
    u.f = f;
    uint32_t    sig = (u.u & 0x7fffff);

    if (f)
        sig |= 0x8000000;

    return (float_gen_t) {
        .kind = fkind(f),
        .neg = signbit(f),
        .exp = exp,
        .sig = { sig, 0 },
    };
}

#endif

#ifdef INCLUDE_FLOAT_64
typedef struct {
    float_64_t  x, y;
} vector_64;


static inline float_gen_t unpack_64(float_64_t f)
{
    int         exp;
    float_64_t  s = name_64(frexp)(f, &exp);
    (void) s;
    union {
        uint64_t        u;
        float_64_t      f;
    } u;
    u.f = f;
    uint64_t    sig = (u.u & 0xfffffffffffffULL);

    if (f)
        sig |= 0x10000000000000ULL;

    return (float_gen_t) {
        .kind = fkind(f),
        .neg = signbit(f),
        .exp = exp,
        .sig = { sig, 0 },
    };
}

#endif

#ifdef INCLUDE_FLOAT_80
typedef struct {
    float_80_t  x, y;
} vector_80;

static inline float_gen_t unpack_80(float_80_t f)
{
    int         exp;
    float_80_t  s = name_80(frexp)(f, &exp);
    (void) s;
    union {
        uint64_t        u[2];
        float_80_t      f;
    } u;
    u.u[0] = 0;
    u.u[1] = 0;
    u.f = f;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t    sig = u.u[0];
#endif

    return (float_gen_t) {
        .kind = fkind(f),
        .neg = signbit(f),
        .exp = exp,
        .sig = { sig, 0 },
    };
}

#endif

#ifdef INCLUDE_FLOAT_128
typedef struct {
    float_128_t  x, y;
} vector_128;

static inline float_gen_t unpack_128(float_128_t f)
{
    int         exp;
    float_128_t  s = name_128(frexp)(f, &exp);
    (void) s;
    union {
        uint64_t        u[2];
        float_128_t     f;
    } u;
    u.u[0] = 0;
    u.u[1] = 0;
    u.f = f;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t    sig0 = u.u[0];
    uint64_t    sig1 = u.u[1] & 0xfff;

    if (f)
        sig1 |= 0x1000;
#endif

    return (float_gen_t) {
        .kind = fkind(f),
        .neg = signbit(f),
        .exp = exp,
        .sig = { sig0, sig1 },
    };
}

#endif

#define abs(a) ((a) < 0 ? -(a) : (a))

static inline void rsl(uint64_t a[2])
{
    a[0] = (a[0] >> 1) | (a[1] << 63);
    a[1] = a[1] >> 1;
}

static inline void lsl(uint64_t a[2])
{
    a[1] = (a[1] << 1) | (a[0] >> 63);
    a[0] = a[0] << 1;
}

static inline float_gen_t error_gen(float_gen_t a, float_gen_t b)
{
    uint64_t    *big, *small;
    uint64_t    sig[2];
    bool        neg = false;

    if (a.kind != b.kind) return (float_gen_t) { .kind = a.kind > b.kind ? a.kind : b.kind };

    if (a.exp < b.exp && b.exp - a.exp < 13) {
        while (a.exp < b.exp) {
            b.exp--;
            lsl(b.sig);
        }
    }
    if (b.exp < a.exp && a.exp - b.exp < 13) {
        while (b.exp < a.exp) {
            a.exp--;
            lsl(a.sig);
        }
    }
    if (a.neg != b.neg) {
        sig[0] = a.sig[0] + b.sig[0];
        sig[1] = a.sig[1] + b.sig[1];
        if (sig[0] < a.sig[0])
            sig[1]++;
        if (sig[1] < a.sig[1])
            neg = true;
    } else {
        if (a.sig[1] < b.sig[1] || (a.sig[1] == b.sig[1] && a.sig[0] < b.sig[0])) {
            big = b.sig;
            small = a.sig;
        } else {
            big = a.sig;
            small = b.sig;
        }
        sig[0] = big[0] - small[0];
        sig[1] = big[1] - small[1];
        if (sig[0] > big[0])
            sig[1]--;
    }
    return (float_gen_t) {
        .kind = 0,
        .neg = neg,
        .exp = abs(a.exp - b.exp),
        .sig = { sig[0], sig[1] }
    };
}

static inline bool error_zero(float_gen_t a)
{
    return a.kind == 0 && a.neg == false && a.exp == 0 && a.sig[0] == 0 && a.sig[1] == 0;
}

static uint32_t
ulp(float_gen_t a)
{
    if (a.kind)
        return 1000000000;
    if (a.neg)
        return 100000000;
    if (a.exp)
        return 10000000;
    if (a.sig[1])
        return 1000000;
    return a.sig[0];
}

#endif /*_MATH_TEST_H_ */
