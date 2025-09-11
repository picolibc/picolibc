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
#include <complex.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Woverflow"
#endif

#define count(a)        (sizeof(a)/sizeof(a[0]))
#define _MATH_CONCAT(a,b)       a ## b
#define MATH_CONCAT(a,b)        _MATH_CONCAT(a, b)
#define _MATH_STRING(a)         #a
#define MATH_STRING(a)          _MATH_STRING(a)

#if __FLT_MANT_DIG__ == 24
#define HAS_BINARY32
typedef float binary32;
typedef complex float cbinary32;
#define CMPLX32(a,b) CMPLXF(FN32(a),FN32(b))
#define creal32(a) crealf(a)
#define cimag32(a) cimagf(a)
#define carg32(a) cargf(a)
#define cabs32(a) cabsf(a)
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
typedef struct {
    uint32_t    r, a;
} cint32_t;

static inline int32_t
bits32(binary32 a)
{
    union { binary32 b; int32_t i; } u;
    u.b = a;
    return u.i;
}

static inline int32_t
ulp32(binary32 a, binary32 b)
{
    if (a == b)
        return 0;

    int32_t     ulp = bits32(a) - bits32(b);
    if (ulp < 0)
        ulp = -ulp;
    return ulp;
}

static inline cint32_t
culp32(cbinary32 a, cbinary32 b)
{
    binary32    a_r = cabs32(a);
    binary32    a_a = carg32(a);
    binary32    b_r = cabs32(b);
    binary32    b_a = carg32(b);
    cint32_t    ulp = {
        .r = ulp32(a_r, b_r),
        .a = ulp32(a_a, b_a),
    };
    return ulp;
}

#endif

#if __DBL_MANT_DIG__ == 53
#define HAS_BINARY64
typedef double binary64;
typedef complex double cbinary64;
#define CMPLX64(a,b) CMPLX(FN64(a),FN64(b))
#define creal64(a) creal(a)
#define cimag64(a) cimag(a)
#define carg64(a) carg(a)
#define cabs64(a) cabs(a)
#define clog64(a) clog(a)
#define TEST_FUNC_64 TEST_FUNC
#define FN64(a)   a
#define FMT64   "% -23.13a"
#define P64(a)  (a)
#elif __LDBL_MANT_DIG__ == 53
#define HAS_BINARY64
typedef long double binary64;
typedef complex long double cbinary64;
#define CMPLX64(a,b) CMPLXL(FN64(a),FN64(b))
#define creal64(a) creall(a)
#define cimag64(a) cimagl(a)
#define carg64(a) cargl(a)
#define cabs64(a) cabsl(a)
#define clog64(a) clogl(a)
#define TEST_FUNC_64 MATH_CONCAT(TEST_FUNC, l)
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

typedef struct {
    uint64_t    r, a;
} cint64_t;

static inline int64_t
bits64(binary64 a)
{
    union { binary64 b; int64_t i; } u;
    u.b = a;
    return u.i;
}

static inline int64_t
ulp64(binary64 a, binary64 b)
{
    if (a == b)
        return 0;

    int64_t     ulp = bits64(a) - bits64(b);
    if (ulp < 0)
        ulp = -ulp;
    return ulp;
}

static inline cint64_t
culp64(cbinary64 a, cbinary64 b)
{
    binary64    a_r = cabs64(a);
    binary64    a_a = carg64(a);
    binary64    b_r = cabs64(b);
    binary64    b_a = carg64(b);
    cint64_t    ulp = {
        .r = ulp64(a_r, b_r),
        .a = ulp64(a_a, b_a),
    };
    return ulp;
}

#endif

#ifdef __SIZEOF_INT128__
typedef __uint128_t uint128_t;
typedef __int128_t int128_t;
#define PRId128 "lld"
#define U128(x) ((unsigned long long) (x))
#endif

#if __LDBL_MANT_DIG__ == 64 && defined(U128)
#define HAS_BINARY80
typedef long double binary80;
typedef complex long double cbinary80;
#define CMPLX80(a,b) CMPLXL(FN80(a),FN80(b))
#define creal80(a) creall(a)
#define cimag80(a) cimagl(a)
#define carg80(a) cargl(a)
#define cabs80(a) cabsl(a)
#define clog80(a) clogl(a)
#define TEST_FUNC_80 MATH_CONCAT(TEST_FUNC, l)
#define FN80(a)   a ## l
#define FMT80   "%La"
#define P80(a)  (a)
#endif

#ifdef HAS_BINARY80

typedef struct {
    binary80    x, y;
} unary80;

typedef struct {
    cbinary80    x, y;
} cunary80;

typedef struct {
    uint128_t    r, a;
} cint128_t;

static inline int128_t
bits80(binary80 a)
{
    union { binary80 b; int128_t i; } u;
    memset(&u, 0, sizeof(u));
    u.b = a;
    return u.i;
}

static inline int128_t
ulp80(binary80 a, binary80 b)
{
    if (a == b)
        return 0;

    int128_t     ulp = bits80(a) - bits80(b);
    if (ulp < 0)
        ulp = -ulp;
    return ulp;
}

static inline cint128_t
culp80(cbinary80 a, cbinary80 b)
{
    binary80    a_r = cabs80(a);
    binary80    a_a = carg80(a);
    binary80    b_r = cabs80(b);
    binary80    b_a = carg80(b);
    cint128_t    ulp = {
        .r = ulp80(a_r, b_r),
        .a = ulp80(a_a, b_a),
    };
    return ulp;
}

#endif

#if __LDBL_MANT_DIG__ == 113 && defined(U128)

#define HAS_BINARY128
typedef long double binary128;
typedef complex long double cbinary128;
#define CMPLX128(a,b) CMPLXL(FN128(a),FN128(b))
#define creal128(a) creall(a)
#define cimag128(a) cimagl(a)
#define carg128(a) cargl(a)
#define cabs128(a) cabsl(a)
#define clog128(a) clogl(a)
#define TEST_FUNC_128 MATH_CONCAT(TEST_FUNC, l)
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

typedef struct {
    uint128_t    r, a;
} cint128_t;

static inline int128_t
bits128(binary128 a)
{
    union { binary128 b; int128_t i; } u;
    memset(&u, 0, sizeof(u));
    u.b = a;
    return u.i;
}

static inline int128_t
ulp128(binary128 a, binary128 b)
{
    if (a == b)
        return 0;

    int128_t     ulp = bits128(a) - bits128(b);
    if (ulp < 0)
        ulp = -ulp;
    return ulp;
}

static inline cint128_t
culp128(cbinary128 a, cbinary128 b)
{
    binary128    a_r = cabs128(a);
    binary128    a_a = carg128(a);
    binary128    b_r = cabs128(b);
    binary128    b_a = carg128(b);
    cint128_t    ulp = {
        .r = ulp128(a_r, b_r),
        .a = ulp128(a_a, b_a),
    };
    return ulp;
}

#endif
