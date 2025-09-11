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

#include <math.h>
#include <float.h>
#include <complex.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

#ifdef __RX__
#define SKIP_DENORM
#endif

#ifdef _RX_PID
#define TEST_CONST
#else
#define TEST_CONST const
#endif

#ifndef MATH_ULP_BINARY32
#define MATH_ULP_BINARY32       1
#endif
#ifndef MATH_ULP_BINARY64
#define MATH_ULP_BINARY64       1
#endif
#ifndef MATH_ULP_BINARY80
#define MATH_ULP_BINARY80       1
#endif
#ifndef MATH_ULP_BINARY128
#define MATH_ULP_BINARY128      1
#endif

#define count(a)        (sizeof(a)/sizeof(a[0]))
#define _MATH_CONCAT(a,b)       a ## b
#define MATH_CONCAT(a,b)        _MATH_CONCAT(a, b)
#define _MATH_STRING(a)         #a
#define MATH_STRING(a)          _MATH_STRING(a)

#ifdef SKIP_DENORM
#define MY_ABS(x)               ({ __typeof(x) __tmp__ = (x); __tmp__ < 0 ? -__tmp__ : __tmp__; })
#define SKIP_DENORM32(x)        (MY_ABS(x) < MIN_BINARY32)
#define SKIP_DENORM64(x)        (MY_ABS(x) < MIN_BINARY64)
#define SKIP_DENORM80(x)        (MY_ABS(x) < MIN_BINARY80)
#define SKIP_DENORM128(x)       (MY_ABS(x) < MIN_BINARY128)
#else
#define SKIP_DENORM32(x)        0
#define SKIP_DENORM64(x)        0
#define SKIP_DENORM80(x)        0
#define SKIP_DENORM128(x)       0
#endif

#define SKIP_CDENORM32(z)       (SKIP_DENORM32(creal32(z)) || SKIP_DENORM32(cimag32(z)))
#define SKIP_CDENORM64(z)       (SKIP_DENORM64(creal64(z)) || SKIP_DENORM64(cimag64(z)))
#define SKIP_CDENORM80(z)       (SKIP_DENORM80(creal80(z)) || SKIP_DENORM80(cimag80(z)))
#define SKIP_CDENORM128(z)       (SKIP_DENORM128(creal128(z)) || SKIP_DENORM128(cimag128(z)))

#define MAX_ULP         9999
#define INVALID_ULP     10000
typedef int32_t ulp_t;
#define PRIdULP         PRId32

#if __FLT_MANT_DIG__ == 24 && !defined(SKIP_BINARY32)
#define HAS_BINARY32
typedef float binary32;
typedef complex float cbinary32;
#define CMPLX32(a,b) CMPLXF(a,b)
#define creal32(a) crealf(a)
#define cimag32(a) cimagf(a)
#define carg32(a) cargf(a)
#define cabs32(a) cabsf(a)
#define nextafter32(x,y) nextafterf(x,y)
#define MIN_BINARY32    FLT_MIN
#define FN32(a) a ## f
#define TEST_FUNC_32 MATH_CONCAT(TEST_FUNC, f)
//#define FMT32        "% -14.8e"
#define FMT32        "% -15.6a"
#define P32(a)  ((double) (a))
#endif

#ifdef HAS_BINARY32
typedef struct {
    binary32    x, y;
} unary32;

typedef struct {
    cbinary32    x, y;
} cunary32;

static inline ulp_t
ulp32 (binary32 a, binary32 b)
{
    if (a == b)
        return 0;
    if (isnan(a) && isnan(b))
        return 0;
    if (isnan(a) || isnan(b)) {
#ifdef __RX__
        printf("RX fails to generate NaN, ignoring\n");
        return 0;
#endif
        return INVALID_ULP;
    }

    ulp_t     ulp = 0;
    while (a != b) {
        a = nextafter32(a,b);
        ulp++;
        if (ulp == MAX_ULP)
            break;
    }
    return ulp;
}

static inline ulp_t
culp32(cbinary32 a, cbinary32 b)
{
    if (a == b)
        return 0;
    binary32    a_r = cabs32(a);
    binary32    d_r = cabs32(a-b);
    return ulp32(a_r + d_r, a_r);
}

#endif

#if __DBL_MANT_DIG__ == 53 && !defined(SKIP_BINARY64)
#define HAS_BINARY64
typedef double binary64;
typedef complex double cbinary64;
#define CMPLX64(a,b) CMPLX(a,b)
#define creal64(a) creal(a)
#define cimag64(a) cimag(a)
#define carg64(a) carg(a)
#define cabs64(a) cabs(a)
#define clog64(a) clog(a)
#define nextafter64(x,y) nextafter(x,y)
#define TEST_FUNC_64 TEST_FUNC
#define MIN_BINARY64    DBL_MIN
#define FN64(a)   a
#define FMT64   "% -23.13a"
#define P64(a)  (a)
#elif __LDBL_MANT_DIG__ == 53 && defined(_TEST_LONG_DOUBLE) && !defined(SKIP_BINARY64)
#define HAS_BINARY64
typedef long double binary64;
typedef complex long double cbinary64;
#define CMPLX64(a,b) CMPLXL(a,b)
#define creal64(a) creall(a)
#define cimag64(a) cimagl(a)
#define carg64(a) cargl(a)
#define cabs64(a) cabsl(a)
#define clog64(a) clogl(a)
#define nextafter64(x,y) nextafterl(x,y)
#define TEST_FUNC_64 MATH_CONCAT(TEST_FUNC, l)
#define MIN_BINARY64    LDBL_MIN
#define FN64(a)   a ## l
#define FMT64   "%La"
#define P64(a)  (a)
#endif

#ifdef HAS_BINARY64

typedef struct {
    binary64    x, y;
} unary64;

typedef struct {
    cbinary64    x, y;
} cunary64;

static inline ulp_t
ulp64(binary64 a, binary64 b)
{
    if (a == b)
        return 0;
    if (isnan(a) && isnan(b))
        return 0;
    if (isnan(a) || isnan(b)) {
#ifdef __RX__
        printf("RX fails to generate NaN, ignoring\n");
        return 0;
#endif
        return INVALID_ULP;
    }

    ulp_t     ulp = 0;
    while (a != b) {
        a = nextafter64(a,b);
        ulp++;
        if (ulp == MAX_ULP)
            break;
    }
    return ulp;
}

static inline
culp64(cbinary64 a, cbinary64 b)
{
    if (a == b)
        return 0;
    binary64    a_r = cabs64(a);
    binary64    d_r = cabs64(a-b);
    return ulp64(a_r + d_r, a_r);
}

#endif

#if __LDBL_MANT_DIG__ == 64 && defined(_TEST_LONG_DOUBLE) && !defined(SKIP_BINARY80)
#define HAS_BINARY80
typedef long double binary80;
typedef complex long double cbinary80;
#define CMPLX80(a,b) CMPLXL(a,b)
#define creal80(a) creall(a)
#define cimag80(a) cimagl(a)
#define carg80(a) cargl(a)
#define cabs80(a) cabsl(a)
#define clog80(a) clogl(a)
#define frexp80(a,e) frexpl(a, e);
#define nextafter80(x,y) nextafterl(x,y)
#define TEST_FUNC_80 MATH_CONCAT(TEST_FUNC, l)
#define MIN_BINARY80    LDBL_MIN
#define FN80(a)   a ## l
#define FMT80   "%La"
#define P80(a)  (a)
#endif

#ifdef HAS_BINARY80

static inline int
skip_binary80(void)
{
#ifdef __m68k__
    volatile long double zero = 0.0L;
    volatile long double one = 1.0L;
    volatile long double check = nextafterl(zero, one);
    if (check + check == zero) {
        printf("m68k emulating long double with double, skipping\n");
        return 1;
    }
#endif
    return 0;
}

typedef struct {
    binary80    x, y;
} unary80;

typedef struct {
    cbinary80    x, y;
} cunary80;

static inline ulp_t
ulp80(binary80 a, binary80 b)
{
    if (a == b)
        return 0;
    if (isnan(a) && isnan(b))
        return 0;
    if (isnan(a) || isnan(b))
        return INVALID_ULP;

    ulp_t     ulp = 0;
    while (a != b) {
        a = nextafterl(a, b);
        ulp++;
        if (ulp == MAX_ULP)
            break;
    }
    return ulp;
}

static inline ulp_t
culp80(cbinary80 a, cbinary80 b)
{
    if (a == b)
        return 0;
    binary80    a_r = cabs80(a);
    binary80    d_r = cabs80(a-b);
    return ulp80(a_r + d_r, a_r);
}

#endif

#if __LDBL_MANT_DIG__ == 113 && defined(_TEST_LONG_DOUBLE) && !defined(SKIP_BINARY128)

#define HAS_BINARY128
typedef long double binary128;
typedef complex long double cbinary128;
#define CMPLX128(a,b) CMPLXL(a,b)
#define creal128(a) creall(a)
#define cimag128(a) cimagl(a)
#define carg128(a) cargl(a)
#define cabs128(a) cabsl(a)
#define nextafter128(x,y) nextafterl(x,y)
#define TEST_FUNC_128 MATH_CONCAT(TEST_FUNC, l)
#define MIN_BINARY128    LDBL_MIN
#define FN128(a)   a ## l
#define FMT128   "%La"
#define P128(a)  (a)
#endif

#ifdef HAS_BINARY128

typedef struct {
    binary128    x, y;
} unary128;

typedef struct {
    cbinary128    x, y;
} cunary128;

static inline ulp_t
ulp128(binary128 a, binary128 b)
{
    if (a == b)
        return 0;
    if (isnan(a) && isnan(b))
        return 0;
    if (isnan(a) || isnan(b))
        return INVALID_ULP;

    ulp_t     ulp = 0;
    while (a != b) {
        a = nextafter128(a,b);
        ulp++;
        if (ulp == MAX_ULP)
            break;
    }
    return ulp;
}

static inline ulp_t
culp128(cbinary128 a, cbinary128 b)
{
    if (a == b)
        return 0;
    binary128   a_r = cabs128(a);
    binary128   d_r = cabs128(a-b);
    return ulp128(a_r + d_r, a_r);
}

#endif
