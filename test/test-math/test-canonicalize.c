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
#define __STDC_WANT_IEC_60559_EXT__
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <fenv.h>

/*
 * A signaling NaN is canonical, and canonicalizel turns it into a quiet
 * NaN, raising FE_INVALID
 */
/* m68k without an FPU emulates long double with double, as other long double tests note */
#if LDBL_MANT_DIG == 64                                    \
    && (defined(__i386__) || defined(__x86_64__)           \
        || (defined(__m68k__) && defined(__HAVE_68881__)))
#define TEST_LD80
#elif LDBL_MANT_DIG == 106
#define TEST_DD
#endif

#if defined(TEST_LD80) || defined(TEST_DD)

/* Exact comparison of the meaningful bits, provided by each format below */
static bool same_ld(long double a, long double b);
static void show_ld(const char *what, long double x);

static int
check_snan(long double x, long double want)
{
    int failures = 0;

    if (!issignaling(x)) {
        printf("sNaN test value is not signaling\n");
        return 1;
    }
    if (!iscanonical(x)) {
        printf("sNaN: iscanonical 0, want 1\n");
        failures++;
    }
    long double r = 0.0L;
    feclearexcept(FE_ALL_EXCEPT);
    int rc = canonicalizel(&r, &x);
#if defined(FE_INVALID) && !defined(__LONG_DOUBLE_NOEXCEPT)
    if (!fetestexcept(FE_INVALID)) {
        printf("sNaN: canonicalizel did not raise FE_INVALID\n");
        failures++;
    }
#endif
    /* The payload must carry over to the quiet NaN */
    if (rc != 0 || !same_ld(r, want)) {
        printf("sNaN: canonicalizel returned %d,", rc);
        show_ld(" got", r);
        show_ld(" want", want);
        printf("\n");
        failures++;
    }
    return failures;
}

#endif

/*
 * iscanonical and canonicalizel on the 80-bit extended formats, which
 * have non-canonical encodings because the integer bit of the
 * significand is explicit.
 *
 * x86 extended: an exponent of zero with the integer bit set is a
 * pseudo-denormal, a non-canonical encoding of the value with exponent
 * one. A non-zero exponent with the integer bit clear (unnormal,
 * pseudo-infinity and pseudo-NaN) is not a valid encoding at all.
 *
 * m68k extended: an exponent of zero with the integer bit set is a
 * normal number. A non-zero, non-maximum exponent with the integer bit
 * clear is an unnormal, a non-canonical encoding of the value which
 * normalizing the significand produces.
 *
 * IBM double-double: a value is the sum of two doubles, hi and lo. It
 * is canonical when hi is that sum rounded to nearest and lo holds the
 * remainder, with lo +0 when there is none, including for zeros,
 * infinities and NaNs. Canonicalizing must not depend on the rounding
 * mode or raise exceptions.
 */

#ifdef TEST_LD80

static long double
make80(uint16_t se, uint64_t m)
{
    unsigned char b[sizeof(long double)] = { 0 };
    long double   x;
#ifdef __m68k__
    b[0] = se >> 8;
    b[1] = se;
    for (int i = 0; i < 8; i++)
        b[4 + i] = m >> (56 - 8 * i);
#else
    for (int i = 0; i < 8; i++)
        b[i] = m >> (8 * i);
    b[8] = se;
    b[9] = se >> 8;
#endif
    memcpy(&x, b, sizeof(x));
    return x;
}

static void
split80(long double x, uint16_t *se, uint64_t *m)
{
    unsigned char b[sizeof(long double)];
    memcpy(b, &x, sizeof(x));
    *m = 0;
#ifdef __m68k__
    *se = (uint16_t)(b[0] << 8 | b[1]);
    for (int i = 0; i < 8; i++)
        *m = *m << 8 | b[4 + i];
#else
    *se = (uint16_t)(b[8] | b[9] << 8);
    for (int i = 7; i >= 0; i--)
        *m = *m << 8 | b[i];
#endif
}

/* Whether an encoding is canonical, independent of the iscanonical under test */
static bool
canonical80(uint16_t se, uint64_t m)
{
    unsigned e = se & 0x7fff;
    bool     ibit = (m >> 63) != 0;
#ifdef __m68k__
    return e == 0 || e == 0x7fff || ibit;
#else
    return e == 0 ? !ibit : ibit;
#endif
}

/*
 * qemu (as of 10.x) misreads m68k unnormal operands, so arithmetic on
 * them gives the wrong value. Report canonicalizel failures for these
 * cases without failing the test until qemu is fixed; set this to 0 to
 * make them count.
 */
#ifndef QEMU_M68K_UNNORMAL_BUG
#define QEMU_M68K_UNNORMAL_BUG 1
#endif

static bool
same_ld(long double a, long double b)
{
    uint16_t ase, bse;
    uint64_t am, bm;
    split80(a, &ase, &am);
    split80(b, &bse, &bm);
    return ase == bse && am == bm;
}

static void
show_ld(const char *what, long double x)
{
    uint16_t se;
    uint64_t m;
    split80(x, &se, &m);
    printf("%s %04x:%016llx", what, se, (unsigned long long)m);
}

#define MUST_CANONICALIZE 0 /* canonicalizel must succeed with the given result */
#define MAY_CANONICALIZE  1 /* no canonical value: may fail, or produce any canonical result */
#define CANONICAL         2 /* already canonical: canonicalizel stores it unchanged */

static const struct {
    uint16_t    se;
    uint64_t    m;
    int         kind;
    uint16_t    want_se;
    uint64_t    want_m;
    const char *name;
} tests[] = {
    /* Canonical on both formats */
    { 0x3fff, 0x8000000000000000ULL, CANONICAL,         0,      0,                     "1.0"                       },
    { 0xbfff, 0x8000000000000000ULL, CANONICAL,         0,      0,                     "-1.0"                      },
    { 0x0000, 0x0000000000000000ULL, CANONICAL,         0,      0,                     "+0"                        },
    { 0x8000, 0x0000000000000000ULL, CANONICAL,         0,      0,                     "-0"                        },
    { 0x0000, 0x0000000000000001ULL, CANONICAL,         0,      0,                     "denormal"                  },
    { 0x7ffe, 0xffffffffffffffffULL, CANONICAL,         0,      0,                     "max"                       },
#ifdef __m68k__
    { 0x0000, 0x8000000000000000ULL, CANONICAL,         0,      0,                     "normal with zero exponent" },
    { 0x7fff, 0x0000000000000000ULL, CANONICAL,         0,      0,                     "inf"                       },
    /* Unnormals: the same value with the significand normalized */
    { 0x3fff, 0x4000000000000000ULL, MUST_CANONICALIZE, 0x3ffe, 0x8000000000000000ULL,
     "unnormal 0.5"                                                                                                },
    { 0xbfff, 0x4000000000000000ULL, MUST_CANONICALIZE, 0xbffe, 0x8000000000000000ULL,
     "unnormal -0.5"                                                                                               },
    { 0x4000, 0x0000000000000001ULL, MUST_CANONICALIZE, 0x3fc1, 0x8000000000000000ULL,
     "unnormal 2^-62"                                                                                              },
    { 0x3fff, 0x0000000000000000ULL, MUST_CANONICALIZE, 0x0000, 0x0000000000000000ULL,
     "unnormal zero"                                                                                               },
#else
    { 0x7fff, 0x8000000000000000ULL, CANONICAL, 0, 0, "inf" },
    { 0xffff, 0x8000000000000000ULL, CANONICAL, 0, 0, "-inf" },
    { 0x7fff, 0xc000000000000000ULL, CANONICAL, 0, 0, "qnan" },
    { 0xffff, 0xc000000000005678ULL, CANONICAL, 0, 0, "-qnan with payload" },
    /* Pseudo-denormals: the same value with exponent one */
    { 0x0000, 0x8000000000000001ULL, MUST_CANONICALIZE, 0x0001, 0x8000000000000001ULL,
      "pseudo-denormal" },
    { 0x8000, 0x8000000000000000ULL, MUST_CANONICALIZE, 0x8001, 0x8000000000000000ULL,
      "-pseudo-denormal" },
    /* Invalid encodings */
    { 0x3fff, 0x4000000000000000ULL, MAY_CANONICALIZE, 0, 0, "unnormal" },
    { 0x3fff, 0x0000000000000000ULL, MAY_CANONICALIZE, 0, 0, "unnormal zero" },
    { 0x7fff, 0x0000000000000000ULL, MAY_CANONICALIZE, 0, 0, "pseudo-infinity" },
    { 0x7fff, 0x4000000000000000ULL, MAY_CANONICALIZE, 0, 0, "pseudo-nan" },
#endif
};

#define NTESTS (sizeof(tests) / sizeof(tests[0]))

int
main(void)
{
    int failures = 0;

    for (unsigned i = 0; i < NTESTS; i++) {
        long double x = make80(tests[i].se, tests[i].m);
        bool        canon = tests[i].kind == CANONICAL;
        uint16_t    se;
        uint64_t    m;

        if ((iscanonical(x) != 0) != canon) {
            printf("%s %04x:%016llx: iscanonical %d, want %d\n", tests[i].name, tests[i].se,
                   (unsigned long long)tests[i].m, iscanonical(x) != 0, canon);
            failures++;
        }

        /* Start with a recognizable value so an unchanged result can be detected */
        long double r = make80(0x4005, 0xc000000000000000ULL);
        int         rc = canonicalizel(&r, &x);
        split80(r, &se, &m);

        switch (tests[i].kind) {
        case CANONICAL:
            if (rc != 0 || se != tests[i].se || m != tests[i].m) {
                printf("%s: canonicalizel returned %d and %04x:%016llx\n", tests[i].name, rc, se,
                       (unsigned long long)m);
                failures++;
            }
            break;
        case MUST_CANONICALIZE:
#if defined(__m68k__) && QEMU_M68K_UNNORMAL_BUG
            if ((rc != 0 || se != tests[i].want_se || m != tests[i].want_m)
                && strncmp(tests[i].name, "unnormal", 8) == 0) {
                printf("%s: canonicalizel returned %d and %04x:%016llx (ignored, qemu bug)\n",
                       tests[i].name, rc, se, (unsigned long long)m);
                break;
            }
#endif
#ifndef __PICOLIBC__
            /* glibc declines to canonicalize x86 pseudo-denormals */
            if (rc != 0 && se == 0x4005 && m == 0xc000000000000000ULL)
                break;
#endif
            if (rc != 0 || se != tests[i].want_se || m != tests[i].want_m) {
                printf("%s: canonicalizel returned %d and %04x:%016llx, want 0 and %04x:%016llx\n",
                       tests[i].name, rc, se, (unsigned long long)m, tests[i].want_se,
                       (unsigned long long)tests[i].want_m);
                failures++;
            }
            break;
        case MAY_CANONICALIZE:
            /* Either fail and leave the result alone, or store a canonical value */
            if (rc != 0) {
                if (se != 0x4005 || m != 0xc000000000000000ULL) {
                    printf("%s: canonicalizel failed but changed the result to %04x:%016llx\n",
                           tests[i].name, se, (unsigned long long)m);
                    failures++;
                }
            } else if (!canonical80(se, m) || (se == tests[i].se && m == tests[i].m)) {
                printf("%s: canonicalizel returned 0 with non-canonical %04x:%016llx\n",
                       tests[i].name, se, (unsigned long long)m);
                failures++;
            }
            break;
        }
    }
    failures
        += check_snan(make80(0x7fff, 0xa000000000001234ULL), make80(0x7fff, 0xe000000000001234ULL));
    printf("%d failures\n", failures);
    return failures != 0;
}

#elif defined(TEST_DD)

static long double
make_dd(double hi, double lo)
{
    double      d[2] = { hi, lo };
    long double x;
    memcpy(&x, d, sizeof(x));
    return x;
}

static void
split_dd(long double x, double *hi, double *lo)
{
    double d[2];
    memcpy(d, &x, sizeof(d));
    *hi = d[0];
    *lo = d[1];
}

/* Bitwise equality */
static bool
same(double a, double b)
{
    return memcmp(&a, &b, sizeof(a)) == 0;
}

static double
dbits(uint64_t u)
{
    double d;
    memcpy(&d, &u, sizeof(d));
    return d;
}

static bool
same_ld(long double a, long double b)
{
    double ahi, alo, bhi, blo;
    split_dd(a, &ahi, &alo);
    split_dd(b, &bhi, &blo);
    return same(ahi, bhi) && same(alo, blo);
}

static void
show_ld(const char *what, long double x)
{
    double hi, lo;
    split_dd(x, &hi, &lo);
    printf("%s (%a, %a)", what, hi, lo);
}

#define ULP    0x1p-52

static const struct {
    double      hi, lo;
    bool        canon;
    double      want_hi, want_lo;
    const char *name;
} tests[] = {
    { 1.0,                     0.0,               true,  0,                       0,           "1.0"                     },
    { -1.0,                    0.0,               true,  0,                       0,           "-1.0"                    },
    { 1.0,                     0x1p-60,           true,  0,                       0,           "(1, 2^-60)"              },
    { 1.0,                     0.25 * ULP,        true,  0,                       0,           "(1, ulp/4)"              },
    { 1.0,                     0.5 * ULP,         true,  0,                       0,           "(1, ulp/2) tie to even"  },
    { DBL_MAX,                 DBL_MAX * 0x1p-54, true,  0,                       0,           "(max, just under ulp/2)" },
    { 0.0,                     0.0,               true,  0,                       0,           "+0"                      },
    { -0.0,                    0.0,               true,  0,                       0,           "-0"                      },
    { (double)INFINITY,        0.0,               true,  0,                       0,           "inf"                     },
    { -(double)INFINITY,       0.0,               true,  0,                       0,           "-inf"                    },
    { __builtin_nan("0x5678"), 0.0,               true,  0,                       0,           "nan with payload"        },
    { 1.0,                     ULP,               false, 1.0 + ULP,               0.0,         "(1, ulp)"                },
    { 1.0,                     0.75 * ULP,        false, 1.0 + ULP,               -0.25 * ULP, "(1, 3ulp/4)"             },
    { 1.0 + ULP,               -0.5 * ULP,        false, 1.0,                     0.5 * ULP,   "(1 + ulp, -ulp/2)"       },
    { -1.0,                    -ULP,              false, -1.0 - ULP,              0.0,         "(-1, -ulp)"              },
    { 0.0,                     1.0,               false, 1.0,                     0.0,         "(0, 1)"                  },
    { 0x1p1000,                0x1p960,           false, 0x1p1000 + 0x1p960,      0.0,         "(2^1000, 2^960)"         },
    { 1.0,                     -0.0,              false, 1.0,                     0.0,         "(1, -0)"                 },
    { 0.0,                     -0.0,              false, 0.0,                     0.0,         "(+0, -0)"                },
    { -0.0,                    -0.0,              false, -0.0,                    0.0,         "(-0, -0)"                },
    { (double)INFINITY,        1.0,               false, (double)INFINITY,        0.0,         "(inf, 1)"                },
    { __builtin_nan("0x5678"), 1.0,               false, __builtin_nan("0x5678"), 0.0,         "(nan with payload, 1)"   },
};

#define NTESTS (sizeof(tests) / sizeof(tests[0]))

static const struct {
    int         mode;
    const char *name;
} modes[] = {
    { FE_TONEAREST,  "nearest"    },
#ifdef FE_UPWARD
    { FE_UPWARD,     "upward"     },
#endif
#ifdef FE_DOWNWARD
    { FE_DOWNWARD,   "downward"   },
#endif
#ifdef FE_TOWARDZERO
    { FE_TOWARDZERO, "towardzero" },
#endif
};

#define NMODES (sizeof(modes) / sizeof(modes[0]))

int
main(void)
{
    int failures = 0;

    for (unsigned j = 0; j < NMODES; j++) {
        for (unsigned i = 0; i < NTESTS; i++) {
            long double x = make_dd(tests[i].hi, tests[i].lo);
            double      want_hi = tests[i].canon ? tests[i].hi : tests[i].want_hi;
            double      want_lo = tests[i].canon ? tests[i].lo : tests[i].want_lo;
            double      hi, lo;

            fesetround(modes[j].mode);
            feclearexcept(FE_ALL_EXCEPT);
            int         canon = iscanonical(x) != 0;
            long double r = make_dd(42.0, 0.0);
            int         rc = canonicalizel(&r, &x);
            int         except = fetestexcept(FE_ALL_EXCEPT);
            fesetround(FE_TONEAREST);
            split_dd(r, &hi, &lo);

            if (canon != tests[i].canon) {
                printf("%s %s: iscanonical %d, want %d\n", tests[i].name, modes[j].name, canon,
                       tests[i].canon);
                failures++;
            }
            if (rc != 0 || !same(hi, want_hi) || !same(lo, want_lo)) {
                printf("%s %s: canonicalizel returned %d and (%a, %a), want 0 and (%a, %a)\n",
                       tests[i].name, modes[j].name, rc, hi, lo, want_hi, want_lo);
                failures++;
            }
            if (except != 0) {
                printf("%s %s: raised exceptions 0x%x\n", tests[i].name, modes[j].name, except);
                failures++;
            }
        }
    }
    failures += check_snan(make_dd(dbits(0x7ff4000000001234ULL), 0.0),
                           make_dd(dbits(0x7ffc000000001234ULL), 0.0));
    printf("%d failures\n", failures);
    return failures != 0;
}

#else

int
main(void)
{
    printf("long double has no non-canonical encodings, skipping\n");
    return 77;
}

#endif
