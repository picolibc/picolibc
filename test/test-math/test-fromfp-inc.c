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
#include "test-math.h"
#include <errno.h>
#include <stdbool.h>

#ifndef FE_DIVBYZERO
#define FE_DIVBYZERO 0
#endif
#ifndef FE_INEXACT
#define FE_INEXACT 0
#endif
#ifndef FE_INVALID
#define FE_INVALID 0
#endif
#ifndef FE_OVERFLOW
#define FE_OVERFLOW 0
#endif
#ifndef FE_UNDERFLOW
#define FE_UNDERFLOW 0
#endif

#ifdef __sh__
#if !(defined(__SH4__) || defined(__SH4_SINGLE__) || defined(__SH4_SINGLE_ONLY__))
#define GDB_SIMULATOR
#endif
#endif

#define MY_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

static const char *
mode_name(unsigned int mode)
{
    switch (mode) {
    case FP_INT_UPWARD:
        return "FP_INT_UPWARD";
    case FP_INT_DOWNWARD:
        return "FP_INT_DOWNWARD";
    case FP_INT_TOWARDZERO:
        return "FP_INT_TOWARDZERO";
    case FP_INT_TONEARESTFROMZERO:
        return "FP_INT_TONEARESTFROMZERO";
    case FP_INT_TONEAREST:
        return "FP_INT_TONEAREST";
    default:
        return "unknown";
    }
}

static const char *
e_to_str(int e)
{
    if (e == 0)
        return "NONE";

#ifdef FE_DIVBYZERO
    if (e == FE_DIVBYZERO)
        return "FE_DIVBYZERO";
#endif
#ifdef FE_OVERFLOW
    if (e == FE_OVERFLOW)
        return "FE_OVERFLOW";
#endif
#ifdef FE_UNDERFLOW
    if (e == FE_UNDERFLOW)
        return "FE_UNDERFLOW";
#endif
#ifdef FE_INEXACT
    if (e == FE_INEXACT)
        return "FE_INEXACT";
#endif
#ifdef FE_INVALID
    if (e == FE_INVALID)
        return "FE_INVALID";
#endif
#if defined(FE_OVERFLOW) && defined(FE_INEXACT)
    if (e == (FE_OVERFLOW | FE_INEXACT))
        return "FE_OVERFLOW|FE_INEXACT";
#endif
#if defined(FE_UNDERFLOW) && defined(FE_INEXACT)
    if (e == (FE_UNDERFLOW | FE_INEXACT))
        return "FE_UNDERFLOW|FE_INEXACT";
#endif
    static char buf[3][128];
    static int  i = 0;
    buf[i][0] = '\0';
    while (e) {
        char *v = NULL;
        char  tmp[24];
#ifdef FE_DIVBYZERO
        if (e & FE_DIVBYZERO) {
            v = "FE_DIVBYZERO";
            e &= ~FE_DIVBYZERO;
        } else
#endif
#ifdef FE_OVERFLOW
            if (e & FE_OVERFLOW) {
            v = "FE_OVERFLOW";
            e &= ~FE_OVERFLOW;
        } else
#endif
#ifdef FE_UNDERFLOW
            if (e & FE_UNDERFLOW) {
            v = "FE_UNDERFLOW";
            e &= ~FE_UNDERFLOW;
        } else
#endif
#ifdef FE_INEXACT
            if (e & FE_INEXACT) {
            v = "FE_INEXACT";
            e &= ~FE_INEXACT;
        } else
#endif
#ifdef FE_INVALID
            if (e & FE_INVALID) {
            v = "FE_INVALID";
            e &= ~FE_INVALID;
        } else
#endif
        {
            snprintf(tmp, sizeof(tmp), "?? 0x%x", e);
            v = tmp;
            e = 0;
        }
#define check_add(s)                                               \
    do {                                                           \
        if (strlen(buf[i]) + strlen(s) + 1 > sizeof(buf[i]))       \
            printf("exception buf overflow %s + %s\n", buf[i], s); \
        else                                                       \
            strcat(buf[i], s);                                     \
    } while (0)
        if (buf[i][0])
            check_add(" | ");
        check_add(v);
    }
    char *ret = buf[i];
    i = (i + 1) % 3;
    return ret;
}

#if defined(HAS_BINARY32) && !defined(SKIP_BINARY32)

static TEST_CONST struct {
    binary32     x, y;
    int          mode;
    unsigned int width;
    int          except;
    int          err;
} test_32_vec[] = {
#define REAL(r32, r64, r80, r80m, r128)     r32
#define REAL_ULP(r32, r64, r80, r80m, r128) r32
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary32(void)
{
    size_t i;
    int    ret = 1;
    int    err, expect_err, except, expect_except;

    printf("test %s\n", MATH_STRING(TEST_FUNC_32));
    for (i = 0; i < count(test_32_vec); i++) {
        if (SKIP_DENORM32(test_32_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        feclearexcept(FE_ALL_EXCEPT);
        errno = 0;
        volatile binary32 y
            = TEST_FUNC_32(test_32_vec[i].x, test_32_vec[i].mode, test_32_vec[i].width);
        err = errno;
        except = fetestexcept(MY_EXCEPT);
        expect_except = 0;
#ifndef BINARY32_NOEXCEPT
        expect_except = test_32_vec[i].except;
#endif
        expect_err = 0;
#if defined(__MATH_ERRNO) || !defined(__PICOLIBC__)
        expect_err = test_32_vec[i].err;
#endif
#ifdef GDB_SIMULATOR
        if (except != expect_except) {
            // printf("ignoring exception error on GDB simulator\n");
            except = expect_except;
        }
#endif
        ulp_t ulp = ulp32(y, test_32_vec[i].y);
        if (ulp != 0 || except != expect_except || err != expect_err) {
            ret = 0;
            printf("%5zu " FMT32 " %s got " FMT32 " %s %s want " FMT32 " %s %s\n", i + 1,
                   P32(test_32_vec[i].x), mode_name(test_32_vec[i].mode), P32(y), e_to_str(except),
                   strerror(err), P32(test_32_vec[i].y), e_to_str(expect_except),
                   strerror(expect_err));
        }
    }
    return ret;
}
#else
#define test_binary32() 1
#endif

#if defined(HAS_BINARY64) && !defined(SKIP_BINARY64)
static TEST_CONST struct {
    binary64     x, y;
    int          mode;
    unsigned int width;
    int          except;
    int          err;
} test_64_vec[] = {
#define REAL(r32, r64, r80, r80m, r128)     r64
#define REAL_ULP(r32, r64, r80, r80m, r128) r64
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary64(void)
{
    size_t i;
    int    ret = 1;
    int    err, expect_err, except, expect_except;

    printf("test %s\n", MATH_STRING(TEST_FUNC_64));
    for (i = 0; i < count(test_64_vec); i++) {
        if (SKIP_DENORM64(test_64_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        feclearexcept(FE_ALL_EXCEPT);
        errno = 0;
        volatile binary64 y
            = TEST_FUNC_64(test_64_vec[i].x, test_64_vec[i].mode, test_64_vec[i].width);
        err = errno;
        except = fetestexcept(MY_EXCEPT);
        expect_except = 0;
#ifndef BINARY64_NOEXCEPT
        expect_except = test_64_vec[i].except;
#endif
        expect_err = 0;
#if defined(__MATH_ERRNO) || !defined(__PICOLIBC__)
        expect_err = test_64_vec[i].err;
#endif
#ifdef GDB_SIMULATOR
        if (except != expect_except) {
            // printf("ignoring exception error on GDB simulator\n");
            except = expect_except;
        }
#endif
        ulp_t ulp = ulp64(y, test_64_vec[i].y);
        if (ulp != 0 || except != expect_except || err != expect_err) {
            ret = 0;
            printf("%5zu " FMT64 " %s got " FMT64 " %s %s want " FMT64 " %s %s\n", i + 1,
                   P64(test_64_vec[i].x), mode_name(test_64_vec[i].mode), P64(y), e_to_str(except),
                   strerror(err), P64(test_64_vec[i].y), e_to_str(expect_except),
                   strerror(expect_err));
        }
    }
    return ret;
}

#else
#define test_binary64() 1
#endif

#if defined(HAS_BINARY80) && !defined(SKIP_BINARY80)
static TEST_CONST struct {
    binary80     x, y;
    int          mode;
    unsigned int width;
    int          except;
    int          err;
} test_80_vec[] = {
#define REAL(r32, r64, r80, r80m, r128)     REAL80(r80, r80m)
#define REAL_ULP(r32, r64, r80, r80m, r128) REAL80(r80, r80m)
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary80(void)
{
    size_t i;
    int    ret = 1;
    int    err, expect_err, except, expect_except;

    if (skip_binary80())
        return ret;
    printf("test %s\n", MATH_STRING(TEST_FUNC_80));
    for (i = 0; i < count(test_80_vec); i++) {
        if (SKIP_DENORM80(test_80_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        feclearexcept(FE_ALL_EXCEPT);
        errno = 0;
        volatile binary80 y
            = TEST_FUNC_80(test_80_vec[i].x, test_80_vec[i].mode, test_80_vec[i].width);
        err = errno;
        except = fetestexcept(MY_EXCEPT);
        expect_except = 0;
#ifndef BINARY80_NOEXCEPT
        expect_except = test_80_vec[i].except;
#endif
        expect_err = 0;
#if defined(__MATH_ERRNO) || !defined(__PICOLIBC__)
        expect_err = test_80_vec[i].err;
#endif
#ifdef GDB_SIMULATOR
        if (except != expect_except) {
            // printf("ignoring exception error on GDB simulator\n");
            except = expect_except;
        }
#endif
        ulp_t ulp = ulp80(y, test_80_vec[i].y);
        if (ulp != 0 || except != expect_except || err != expect_err) {
            ret = 0;
            printf("%5zu " FMT80 " %s got " FMT80 " %s %s want " FMT80 " %s %s\n", i + 1,
                   P80(test_80_vec[i].x), mode_name(test_80_vec[i].mode), P80(y), e_to_str(except),
                   strerror(err), P80(test_80_vec[i].y), e_to_str(expect_except),
                   strerror(expect_err));
        }
    }
    printf("  max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary80() 1
#endif

#if defined(HAS_BINARY128) && !defined(SKIP_BINARY128)
static TEST_CONST struct {
    binary128    x, y;
    int          mode;
    unsigned int width;
    int          except;
    int          err;
} test_128_vec[] = {
#define REAL(r32, r64, r80, r80m, r128)     r128
#define REAL_ULP(r32, r64, r80, r80m, r128) r128
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary128(void)
{
    size_t i;
    int    ret = 1;
    int    err, expect_err, except, expect_except;

    printf("test %s\n", MATH_STRING(TEST_FUNC_128));
    for (i = 0; i < count(test_128_vec); i++) {
        if (SKIP_DENORM128(test_128_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        feclearexcept(FE_ALL_EXCEPT);
        errno = 0;
        volatile binary128 y
            = TEST_FUNC_128(test_128_vec[i].x, test_128_vec[i].mode, test_128_vec[i].width);
        err = errno;
        except = fetestexcept(MY_EXCEPT);
        expect_except = 0;
#ifndef BINARY128_NOEXCEPT
        expect_except = test_128_vec[i].except;
#endif
        expect_err = 0;
#if defined(__MATH_ERRNO) || !defined(__PICOLIBC__)
        expect_err = test_128_vec[i].err;
#endif
#ifdef GDB_SIMULATOR
        if (except != expect_except) {
            // printf("ignoring exception error on GDB simulator\n");
            except = expect_except;
        }
#endif
        ulp_t ulp = ulp128(y, test_128_vec[i].y);
        if (ulp != 0 || except != expect_except || err != expect_err) {
            ret = 0;
            printf("%5zu " FMT128 " %s got " FMT128 " %s %s want " FMT128 " %s %s\n", i + 1,
                   P128(test_128_vec[i].x), mode_name(test_128_vec[i].mode), P128(y),
                   e_to_str(except), strerror(err), P128(test_128_vec[i].y),
                   e_to_str(expect_except), strerror(expect_err));
        }
    }
    printf("  max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary128() 1
#endif

int
main(void)
{
    int ret = 0;
    if (!test_binary32())
        ret = 1;
    if (!test_binary64())
        ret = 1;
    if (!test_binary80())
        ret = 1;
    if (!test_binary128())
        ret = 1;

    if (ret)
        printf("FAILED\n");
    else
        printf("PASSED\n");
    return ret;
}
