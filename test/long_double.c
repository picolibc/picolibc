/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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

#define _GNU_SOURCE
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#ifdef _TEST_LONG_DOUBLE

static long double max_error;

bool
within_error(long double expect, long double result, long double error)
{
    long double difference;
    long double e = 1.0L;

    if (isnan(expect) && isnan(result))
        return true;

    if (expect == result)
        return true;

    if (expect != 0)
        e = scalbnl(1.0L, -ilogbl(expect));

    difference = fabsl(expect - result) * e;

    if (difference > max_error)
        max_error = difference;

    return difference <= error;
}

int
check_long_double(const char *name, int i, long double prec, long double expect, long double result)
{
    if (!within_error(expect, result, prec)) {
        long double diff = fabsl(expect - result);
#ifdef __PICOLIBC__
#ifdef _WANT_IO_LONG_DOUBLE
        printf("%s test %d got %La expect %La diff %La\n", name, i, result, expect, diff);
#else
        printf("%s test %d got %a expect %a diff %a\n", name, i, (double) result, (double) expect, (double) diff);
#endif
#else
//        printf("%s test %d got %.33Lg expect %.33Lg diff %.33Lg\n", name, i, result, expect, diff);
        printf("%s test %d got %La expect %La diff %La\n", name, i, result, expect, diff);
#endif
        return 1;
    }
    return 0;
}

int
check_long_long(const char *name, int i, long long expect, long long result)
{
    if (expect != result) {
        long long diff = expect - result;
        printf("%s test %d got %lld expect %lld diff %lld\n", name, i, result, expect, diff);
        return 1;
    }
    return 0;
}

typedef const struct {
    const char *name;
    int (*test)(void);
} long_double_test_t;

typedef const struct {
    int line;
    long double x;
    long double y;
} long_double_test_f_f_t;

typedef const struct {
    int line;
    long double x0;
    long double x1;
    long double y;
} long_double_test_f_ff_t;

typedef const struct {
    int line;
    long double x0;
    long double x1;
    long double x2;
    long double y;
} long_double_test_f_fff_t;

typedef const struct {
    int line;
    long double x0;
    int x1;
    long double y;
} long_double_test_f_fi_t;

typedef const struct {
    int line;
    long double x;
    long long y;
} long_double_test_i_f_t;

/*
 * sqrtl is "exact", but can have up to one bit of error as it might
 * not have enough guard bits to correctly perform rounding, leading
 * to some answers getting rounded to an even value instead of the
 * (more accurate) odd value
 */
#define FMA_PREC 0
#if LDBL_MANT_DIG == 64
#define DEFAULT_PREC 0x1p-55L
#define SQRTL_PREC 0x1.0p-63L
#define FULL_LONG_DOUBLE
#elif LDBL_MANT_DIG == 113
#define FULL_LONG_DOUBLE
#define DEFAULT_PREC 0x1p-105L
#define SQRTL_PREC 0x1.0p-112L
#elif LDBL_MANT_DIG == 106
#define DEFAULT_PREC 0x1p-97L
#define SQRTL_PREC 0x1.0p-105L
#define PART_LONG_DOUBLE
#elif LDBL_MANT_DIG == 53
#define DEFAULT_PREC 0x1p-48L
#define SQRTL_PREC 0x1.0p-52L
#else
#error unsupported long double
#endif

#define HYPOTL_PREC     SQRTL_PREC
#define CBRTL_PREC      SQRTL_PREC
#define CEILL_PREC      0
#define FLOORL_PREC     0
#define LOGBL_PREC      0
#define RINTL_PREC      0
#define FMINL_PREC      0
#define FMAXL_PREC      0
#define SCALBNL_PREC    0
#define SCALBL_PREC     0
#define LDEXPL_PREC     0
#define COPYSIGNL_PREC  0
#define NEARBYINTL_PREC 0
#define ROUNDL_PREC     0
#define TRUNCL_PREC     0

#include "long_double_vec.h"

#if !defined(__PICOLIBC__) || (defined(_WANT_IO_LONG_DOUBLE) && (defined(TINY_STDIO) || defined(FLOATING_POINT)))
#define TEST_IO_LONG_DOUBLE
#endif

#if defined(__PICOLIBC__) && defined(__m68k__) && !defined(TINY_STDIO)
#undef TEST_IO_LONG_DOUBLE
#endif

#ifdef TEST_IO_LONG_DOUBLE
static long double vals[] = {
    1.0L,
    0x1.8p0L,
    3.141592653589793238462643383279502884197169L,
    0.0L,
    1.0L/0.0L,
    0.0L/0.0L,
};

#define NVALS   (sizeof(vals)/sizeof(vals[0]))

static long double
naive_strtold(const char *buf)
{
    long double v = 0.0L;
    long exp = 0;
    long double frac_mul = 1.0L / 16.0L;
    long exp_sign = 1;
    char c;
    enum {
        LDOUBLE_INT,
        LDOUBLE_FRAC,
        LDOUBLE_EXP,
    } state = LDOUBLE_INT;

    if (strncmp(buf, "0x", 2) != 0)
        return -(long double)INFINITY;
    buf += 2;
    while ((c = *buf++)) {
        int digit;
        switch (c) {
        case '.':
            if (state == LDOUBLE_INT) {
                state = LDOUBLE_FRAC;
                continue;
            }
            return -(long double)INFINITY;
        case 'p':
            if (state == LDOUBLE_INT || state == LDOUBLE_FRAC) {
                state = LDOUBLE_EXP;
                continue;
            }
            return -(long double)INFINITY;
        case '-':
            if (state == LDOUBLE_EXP) {
                exp_sign = -1;
                continue;
            }
            return -(long double)INFINITY;
        case '+':
            if (state == LDOUBLE_EXP) {
                exp_sign = 1;
                continue;
            }
            return -(long double)INFINITY;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            digit = c - '0';
            break;
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            if (state == LDOUBLE_INT || state == LDOUBLE_FRAC) {
                digit = c - 'A' + 10;
                break;
            }
            return -(long double)INFINITY;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            if (state == LDOUBLE_INT || state == LDOUBLE_FRAC) {
                digit = c - 'a' + 10;
                break;
            }
            return -(long double)INFINITY;
        default:
            return -(long double)INFINITY;
        }
        switch (state) {
        case LDOUBLE_INT:
            v = v * 16.0L + digit;
            break;
        case LDOUBLE_FRAC:
            v = v + digit * frac_mul;
            frac_mul *= 1.0L / 16.0L;
            break;
        case LDOUBLE_EXP:
            exp = exp * 10 + digit;
            break;
        }
    }
    return ldexpl(v, exp * exp_sign);
}

static int
test_io(void)
{
    int e;
    int result = 0;
    char buf[80];
    unsigned i;
    char *end;

    for (e = __LDBL_MIN_EXP__ - __LDBL_MANT_DIG__; e <= __LDBL_MAX_EXP__; e++)
    {
        long double v, r;
        for (i = 0; i < NVALS; i++) {
            v = ldexpl(vals[i], e);
            sprintf(buf, "%La", v);
            if (isinf(v)) {
                if (strcmp(buf, "inf") != 0) {
                    printf("test_io i %d val %La exp %d: is %s should be inf\n", i, vals[i], e, buf);
                    result++;
                }
            } else if (isnan(v)) {
                if (strcmp(buf, "nan") != 0) {
                    printf("test_io is %s should be nan\n", buf);
                    result++;
                }
            } else {
                r = naive_strtold(buf);
                if (v != r) {
                    printf("test_io naive i %d val %La exp %d: \"%s\", is %La should be %La\n", i, vals[i], e, buf, r, v);
                    result++;
                }
            }
            sscanf(buf, "%Lf", &r);
            if (v != r && !(isnan(v) && isnan(r))) {
                printf("test_io scanf i %d val %La exp %d: \"%s\", is %La should be %La\n", i, vals[i], e, buf, r, v);
                result++;
            }
            r = strtold(buf, &end);
            if ((v != r && !(isnan(v) && isnan(r)))|| end != buf + strlen(buf)) {
                printf("test_io strtold i %d val %La exp %d: \"%s\", is %La should be %La\n", i, vals[i], e, buf, r, v);
                result++;
            }
        }
    }
    return result;
}
#endif

int main(void)
{
    int result = 0;
    unsigned int i;

    printf("LDBL_MANT_DIG %d\n", LDBL_MANT_DIG);
#ifdef __m68k__
    volatile long double zero = 0.0L;
    volatile long double one = 1.0L;
    volatile long double check = nextafterl(zero, one);
    if (check + check == zero) {
        printf("m68k emulating long double with double, skipping\n");
        return 77;
    }
#endif
#ifdef TEST_IO_LONG_DOUBLE
    result += test_io();
#endif
    for (i = 0; i < sizeof(long_double_tests) / sizeof(long_double_tests[0]); i++) {
        result += long_double_tests[i].test();
    }
    return result != 0;
}

#else
int main(void)
{
    printf("no long double support\n");
    return 0;
}
#endif
