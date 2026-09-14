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
#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <unistd.h>
#include <string.h>

#ifdef WIDE_LD
typedef long double wide_t;
typedef __uint128_t uintwide_t;
#define WIDE_FMT "%La"
#else
typedef double   wide_t;
typedef uint64_t uintwide_t;
#define WIDE_FMT "%a"
#endif

#ifdef NARROW_D
typedef double narrow_t;
#else
typedef float narrow_t;
#endif

#define T_MIN_EXP(T)                                                                   \
    _Generic((T)1, float: FLT_MIN_EXP, double: DBL_MIN_EXP, long double: LDBL_MIN_EXP)
#define T_MAX_EXP(T)                                                                   \
    _Generic((T)1, float: FLT_MAX_EXP, double: DBL_MAX_EXP, long double: LDBL_MAX_EXP)
#define T_MANT_DIG(T)                                                                     \
    _Generic((T)1, float: FLT_MANT_DIG, double: DBL_MANT_DIG, long double: LDBL_MANT_DIG)

#define NARROW_MIN_EXP  T_MIN_EXP(narrow_t)
#define NARROW_MAX_EXP  T_MAX_EXP(narrow_t)
#define NARROW_MANT_DIG T_MANT_DIG(narrow_t)

#define WIDE_MIN_EXP    T_MIN_EXP(wide_t)
#define WIDE_MAX_EXP    T_MAX_EXP(wide_t)
#define WIDE_MANT_DIG   T_MANT_DIG(wide_t)

#define LDEXP(x, e)     _Generic(x, float: ldexpf(x, e), double: ldexp(x, e), long double: ldexpl(x, e))

#define DRAND48()                                                                            \
    _Generic((wide_t)1, float: (float)drand48(), double: drand48(), long double: ldrand48())
#define FMA(a, b, c)                                                                    \
    _Generic(a, float: fmaf(a, b, c), double: fma(a, b, c), long double: fmal(a, b, c))
#define FFMA(a, b, c)                                                                     \
    _Generic(a, float: fmaf(a, b, c), double: ffma(a, b, c), long double: ffmal(a, b, c))
#define DFMA(a, b, c)                                                                            \
    _Generic(a, float: (double)fmaf(a, b, c), double: fma(a, b, c), long double: dfmal(a, b, c))
#define NFMA(a, b, c) _Generic((narrow_t)1, float: FFMA(a, b, c), double: DFMA(a, b, c))

#define FADD(a, b)    _Generic(a, float: (a) + (b), double: fadd(a, b), long double: faddl(a, b))

#define DADD(a, b)    _Generic(a, float: (a) + (b), double: (a) + (b), long double: daddl(a, b))

#define NADD(a, b)    _Generic((narrow_t)1, float: FADD(a, b), double: DADD(a, b))

#define FSUB(a, b)    _Generic(a, float: (a) - (b), double: fsub(a, b), long double: fsubl(a, b))

#define DSUB(a, b)    _Generic(a, float: (a) - (b), double: (a) + (b), long double: dsubl(a, b))

#define NSUB(a, b)    _Generic((narrow_t)1, float: FSUB(a, b), double: DSUB(a, b))

#define FMUL(a, b)    _Generic(a, float: (a) * (b), double: fmul(a, b), long double: fmull(a, b))

#define DMUL(a, b)    _Generic(a, float: (a) * (b), double: (a) * (b), long double: dmull(a, b))

#define NMUL(a, b)    _Generic((narrow_t)1, float: FMUL(a, b), double: DMUL(a, b))

#define FDIV(a, b)    _Generic(a, float: (a) / (b), double: fdiv(a, b), long double: fdivl(a, b))

#define DDIV(a, b)    _Generic(a, float: (a) / (b), double: (a) / (b), long double: ddivl(a, b))

#define NDIV(a, b)    _Generic((narrow_t)1, float: FDIV(a, b), double: DDIV(a, b))

#define SQRT(a)       _Generic(a, float: sqrtf(a), double: sqrt(a), long double: sqrtl(a))

#define FSQRT(a)      _Generic(a, float: sqrtf(a), double: fsqrt(a), long double: fsqrtl(a))

#define DSQRT(a)      _Generic(a, float: sqrtf(a), double: sqrt(a), long double: dsqrtl(a))

#define NSQRT(a)      _Generic((narrow_t)1, float: FSQRT(a), double: DSQRT(a))

static inline long double
ldrand48(void)
{
    return (long double)drand48() * (long double)drand48();
}

static inline int
rand_int(int max)
{
    int pow = 1;
    while (pow < max) {
        pow <<= 1;
    }
    for (;;) {
        int v = (random() & (pow - 1));
        if (v < max)
            return v;
    }
}

static inline int
rand_e(int min_e, int max_e)
{
    return min_e + rand_int(max_e - min_e);
}

static inline wide_t
rand_double(int e)
{
    return LDEXP(DRAND48(), e);
}

typedef struct {
    const char *name;
    narrow_t    (*single_round)(wide_t, wide_t, wide_t);
    narrow_t    (*double_round)(wide_t, wide_t, wide_t);
    size_t      differ;
    size_t      same;
} funcs_t;

static narrow_t
fadd_dround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    return (narrow_t)(a + b);
}

static narrow_t
fsub_dround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    return (narrow_t)(a - b);
}

static narrow_t
fmul_dround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    return (narrow_t)(a * b);
}

static narrow_t
fdiv_dround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    return (narrow_t)(a / b);
}

static narrow_t
ffma_dround(wide_t a, wide_t b, wide_t c)
{
    return (narrow_t)FMA(a, b, c);
}

static narrow_t
fsqrt_dround(wide_t a, wide_t b, wide_t c)
{
    (void)b;
    (void)c;
    return (narrow_t)SQRT(a);
}

static narrow_t
fadd_sround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    return NADD(a, b);
}

static narrow_t
fsub_sround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    return NSUB(a, b);
}

static narrow_t
fmul_sround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    narrow_t r = NMUL(a, b);
    if (r == 0)
        return (narrow_t)INFINITY;
    return r;
}

static narrow_t
fdiv_sround(wide_t a, wide_t b, wide_t c)
{
    (void)c;
    narrow_t r = NDIV(a, b);
    if (r == 0)
        return (narrow_t)INFINITY;
    return r;
}

static narrow_t
ffma_sround(wide_t a, wide_t b, wide_t c)
{
    narrow_t r = NFMA(a, b, c);
    if (r == 0)
        return (narrow_t)INFINITY;
    return r;
}

static narrow_t
fsqrt_sround(wide_t a, wide_t b, wide_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return NSQRT(a);
}

static funcs_t funcs[] = {
    { .name = "fadd",  .single_round = fadd_sround,  .double_round = fadd_dround  },
    { .name = "fmul",  .single_round = fmul_sround,  .double_round = fmul_dround  },
    { .name = "fsub",  .single_round = fsub_sround,  .double_round = fsub_dround  },
    { .name = "fdiv",  .single_round = fdiv_sround,  .double_round = fdiv_dround  },
    { .name = "ffma",  .single_round = ffma_sround,  .double_round = ffma_dround  },
    { .name = "fsqrt", .single_round = fsqrt_sround, .double_round = fsqrt_dround },
};

#define NFUNCS       (sizeof(funcs) / sizeof(funcs[0]))

#define TEST_VECTORS 64

uintwide_t
touint(wide_t d)
{
    union {
        wide_t     d;
        uintwide_t u;
    } u;
    memset(&u, 0, sizeof(u));
    u.d = d;
    return u.u;
}

wide_t
towide(uintwide_t uwide)
{
    union {
        wide_t     d;
        uintwide_t u;
    } u;
    memset(&u, 0, sizeof(u));
    u.u = uwide;
    return u.d;
}

int
main(int argc, char **argv)
{
    size_t try = 0;
    size_t f;
    size_t test_vectors = TEST_VECTORS;
    size_t same, differ;
    long   seed;

    if (argc > 1)
        test_vectors = strtoul(argv[1], NULL, 10);
    if (argc > 2)
        seed = strtoll(argv[2], NULL, 10);
    else
        seed = getpid();

    srand48(seed);

    for (f = 0; f < NFUNCS; f++) {

        same = 0;
        differ = 0;

        while (same < test_vectors || differ < test_vectors) {
            int        a_e = rand_e(NARROW_MIN_EXP + NARROW_MANT_DIG, NARROW_MAX_EXP);
            int        b_e = rand_e(a_e - NARROW_MANT_DIG, a_e + NARROW_MANT_DIG);
            int        c_e = rand_e(a_e + b_e - NARROW_MANT_DIG, a_e + b_e + NARROW_MANT_DIG);
            wide_t     a = rand_double(a_e);
            wide_t     b = rand_double(b_e);
            wide_t     c = rand_double(c_e);
            narrow_t   double_round, single_round;
            narrow_t   double_round_start, single_round_start;
            uintwide_t u;
            uintwide_t start = touint(a);
            uintwide_t stop, step;

            double_round_start = funcs[f].double_round(a, b, c);
            single_round_start = funcs[f].single_round(a, b, c);
            if (!finite(double_round_start) || !finite(single_round_start))
                continue;

            step = ((uintwide_t)1 << (WIDE_MANT_DIG - 3));

            while (step > 16) {
#if 0
                printf("Starting boundary scan step %llu for %s " WIDE_FMT " " WIDE_FMT " " WIDE_FMT "\n",
                       (long long) step, funcs[f].name, a, b, c);
#endif

                for (;;) {
                    stop = start + step;
                    a = towide(stop);
                    double_round = funcs[f].double_round(a, b, c);
                    single_round = funcs[f].single_round(a, b, c);

                    if (double_round != double_round_start || single_round != single_round_start)
                        break;
                    start = stop;
                }
                step >>= 1;
            }
#if 0
            printf("Starting full scan for %s " WIDE_FMT "..." WIDE_FMT " " WIDE_FMT " " WIDE_FMT "\n", funcs[f].name,
                   towide(start), towide(stop), b, c);
#endif

            for (u = start; u <= stop; u++) {
                a = towide(u);
                double_round = funcs[f].double_round(a, b, c);
                single_round = funcs[f].single_round(a, b, c);

                if (!finite(double_round) || !finite(single_round))
                    break;

                if (double_round != single_round) {
                    if (same >= differ) {
                        printf("    { .a = " WIDE_FMT ", .b = " WIDE_FMT ", .c = " WIDE_FMT
                               " } /* %s differ */\n",
                               a, b, c, funcs[f].name);
                        ++differ;
                    }
                    break;
                } else {
                    if (differ > same) {
                        printf("    { .a = " WIDE_FMT ", .b = " WIDE_FMT ", .c = " WIDE_FMT
                               " } /* %s same */\n",
                               a, b, c, funcs[f].name);
                        ++same;
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
