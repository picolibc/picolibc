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

/*
 * Check that the <tgmath.h> macros select the function required by
 * C23 7.27: by result type for every macro and argument type
 * combination, by value against direct calls, and that each argument
 * is evaluated once. With picolibc, also check the macros for the
 * extension functions.
 */

#define _GNU_SOURCE
#include <tgmath.h>
#include <float.h>
#include <stdio.h>

/* Mixing float and double arguments is the point of this test */
#pragma GCC diagnostic ignored "-Wdouble-promotion"

#if __STDC_VERSION_TGMATH_H__ != 202311L
#error __STDC_VERSION_TGMATH_H__ has the wrong value
#endif

static int errors;

#define CHECK_TYPE(expr, T)                                         \
    do {                                                            \
        if (!_Generic((expr), T: 1, default: 0)) {                  \
            printf("line %d: %s is not %s\n", __LINE__, #expr, #T); \
            errors++;                                               \
        }                                                           \
    } while (0)

/*
 * Compare in a separate function so the tests don't contain any
 * floating point branches. test_tables is large enough that GCC uses
 * 32-bit FBcc displacements on ColdFire, which QEMU fails to decode.
 * Converting to long double is exact, so this works for every type.
 */
static __attribute__((noinline)) int
same(long double a, long double b)
{
    return a == b || (isnan(a) && isnan(b));
}

#define SAME(a, b) same(a, b)

#define CHECK_VAL(expr, ref)                                                \
    do {                                                                    \
        if (!SAME(expr, ref)) {                                             \
            printf("line %d: %s differs from %s\n", __LINE__, #expr, #ref); \
            errors++;                                                       \
        }                                                                   \
    } while (0)

#define CHECK_CVAL(expr, ref)                                                   \
    do {                                                                        \
        if (!SAME(creal(expr), creal(ref)) || !SAME(cimag(expr), cimag(ref))) { \
            printf("line %d: %s differs from %s\n", __LINE__, #expr, #ref);     \
            errors++;                                                           \
        }                                                                       \
    } while (0)

/* Other C libraries always provide the long double functions */
#if defined(__HAVE_LONG_DOUBLE_MATH) || !defined(__PICOLIBC__)
#define HAVE_LONG_DOUBLE_MATH
#endif

/*
 * tgmath.h always uses the long double functions for long double
 * arguments, but picolibc only implements all of them when it has
 * long double math. Elsewhere, only check the types, which doesn't
 * reference the functions.
 */
#ifdef HAVE_LONG_DOUBLE_MATH
#define CHECK_LVAL(expr, ref)  CHECK_VAL(expr, ref)
#define CHECK_LCVAL(expr, ref) CHECK_CVAL(expr, ref)
#else
#define CHECK_LVAL(expr, ref)  ((void)0)
#define CHECK_LCVAL(expr, ref) ((void)0)
#endif

static volatile float       xf = 0.5f;
static volatile double      xd = 0.5;
static volatile long double xl = 0.5L;
static volatile int         xi = 2;
static volatile float _Complex cf = 0.5f + 0.25f * I;
static volatile double _Complex cd = 0.5 + 0.25 * I;
static volatile long double _Complex cl = 0.5L + 0.25L * I;

#define TEST_REAL_COMPLEX1(fn, cf_)               \
    do {                                          \
        CHECK_TYPE(fn(xf), float);                \
        CHECK_TYPE(fn(xd), double);               \
        CHECK_TYPE(fn(xl), long double);          \
        CHECK_TYPE(fn(xi), double);               \
        CHECK_TYPE(fn(cf), float _Complex);       \
        CHECK_TYPE(fn(cd), double _Complex);      \
        CHECK_TYPE(fn(cl), long double _Complex); \
        CHECK_VAL(fn(xf), (fn##f)(xf));           \
        CHECK_VAL(fn(xd), (fn)(xd));              \
        CHECK_LVAL(fn(xl), (fn##l)(xl));          \
        CHECK_VAL(fn(xi), (fn)((double)xi));      \
        CHECK_CVAL(fn(cf), (cf_##f)(cf));         \
        CHECK_CVAL(fn(cd), (cf_)(cd));            \
        CHECK_LCVAL(fn(cl), (cf_##l)(cl));        \
    } while (0)

#define TEST_REAL1(fn)                       \
    do {                                     \
        CHECK_TYPE(fn(xf), float);           \
        CHECK_TYPE(fn(xd), double);          \
        CHECK_TYPE(fn(xl), long double);     \
        CHECK_TYPE(fn(xi), double);          \
        CHECK_VAL(fn(xf), (fn##f)(xf));      \
        CHECK_VAL(fn(xd), (fn)(xd));         \
        CHECK_LVAL(fn(xl), (fn##l)(xl));     \
        CHECK_VAL(fn(xi), (fn)((double)xi)); \
    } while (0)

#define TEST_REAL1_INT(fn, T)            \
    do {                                 \
        CHECK_TYPE(fn(xf), T);           \
        CHECK_TYPE(fn(xd), T);           \
        CHECK_TYPE(fn(xl), T);           \
        CHECK_TYPE(fn(xi), T);           \
        CHECK_VAL(fn(xf), (fn##f)(xf));  \
        CHECK_VAL(fn(xd), (fn)(xd));     \
        CHECK_LVAL(fn(xl), (fn##l)(xl)); \
    } while (0)

#define TEST_REAL2(fn)                           \
    do {                                         \
        CHECK_TYPE(fn(xf, xf), float);           \
        CHECK_TYPE(fn(xf, xd), double);          \
        CHECK_TYPE(fn(xd, xf), double);          \
        CHECK_TYPE(fn(xf, xl), long double);     \
        CHECK_TYPE(fn(xl, xd), long double);     \
        CHECK_TYPE(fn(xf, xi), double);          \
        CHECK_TYPE(fn(xi, xi), double);          \
        CHECK_TYPE(fn(xi, xl), long double);     \
        CHECK_VAL(fn(xf, xf), (fn##f)(xf, xf));  \
        CHECK_VAL(fn(xf, xd), (fn)(xf, xd));     \
        CHECK_LVAL(fn(xf, xl), (fn##l)(xf, xl)); \
        CHECK_VAL(fn(xi, xf), (fn)(xi, xf));     \
    } while (0)

#define TEST_COMPLEX_REAL(fn)            \
    do {                                 \
        CHECK_TYPE(fn(xf), float);       \
        CHECK_TYPE(fn(xd), double);      \
        CHECK_TYPE(fn(xl), long double); \
        CHECK_TYPE(fn(xi), double);      \
        CHECK_TYPE(fn(cf), float);       \
        CHECK_TYPE(fn(cd), double);      \
        CHECK_TYPE(fn(cl), long double); \
        CHECK_VAL(fn(cf), (fn##f)(cf));  \
        CHECK_VAL(fn(cd), (fn)(cd));     \
        CHECK_LVAL(fn(cl), (fn##l)(cl)); \
    } while (0)

#define TEST_COMPLEX_COMPLEX(fn)                  \
    do {                                          \
        CHECK_TYPE(fn(xf), float _Complex);       \
        CHECK_TYPE(fn(xd), double _Complex);      \
        CHECK_TYPE(fn(xl), long double _Complex); \
        CHECK_TYPE(fn(xi), double _Complex);      \
        CHECK_TYPE(fn(cf), float _Complex);       \
        CHECK_TYPE(fn(cd), double _Complex);      \
        CHECK_TYPE(fn(cl), long double _Complex); \
        CHECK_CVAL(fn(cf), (fn##f)(cf));          \
        CHECK_CVAL(fn(cd), (fn)(cd));             \
        CHECK_LCVAL(fn(cl), (fn##l)(cl));         \
    } while (0)

/*
 * GCC inlines sqrt as fsqrt with a floating point branch to the errno
 * path, so keep it out of test_tables for the same reason as same().
 */
static void
test_sqrt(void)
{
    TEST_REAL_COMPLEX1(sqrt, csqrt);
}

static void
test_tables(void)
{
    /* One-argument functions with real and complex versions */
    TEST_REAL_COMPLEX1(acos, cacos);
    TEST_REAL_COMPLEX1(asin, casin);
    TEST_REAL_COMPLEX1(atan, catan);
    TEST_REAL_COMPLEX1(acosh, cacosh);
    TEST_REAL_COMPLEX1(asinh, casinh);
    TEST_REAL_COMPLEX1(atanh, catanh);
    TEST_REAL_COMPLEX1(cos, ccos);
    TEST_REAL_COMPLEX1(sin, csin);
    TEST_REAL_COMPLEX1(tan, ctan);
    TEST_REAL_COMPLEX1(cosh, ccosh);
    TEST_REAL_COMPLEX1(sinh, csinh);
    TEST_REAL_COMPLEX1(tanh, ctanh);
    TEST_REAL_COMPLEX1(exp, cexp);
    TEST_REAL_COMPLEX1(log, clog);

    /* One-argument real-only functions returning a floating type */
    TEST_REAL1(cbrt);
    TEST_REAL1(ceil);
    TEST_REAL1(erf);
    TEST_REAL1(erfc);
    TEST_REAL1(exp2);
    TEST_REAL1(expm1);
    TEST_REAL1(floor);
    TEST_REAL1(lgamma);
    TEST_REAL1(log10);
    TEST_REAL1(log1p);
    TEST_REAL1(log2);
    TEST_REAL1(logb);
    TEST_REAL1(nearbyint);
    TEST_REAL1(rint);
    TEST_REAL1(round);
    TEST_REAL1(tgamma);
    TEST_REAL1(trunc);
    TEST_REAL1(acospi);
    TEST_REAL1(asinpi);
    TEST_REAL1(atanpi);
    TEST_REAL1(cospi);
    TEST_REAL1(exp10);
    TEST_REAL1(exp10m1);
    TEST_REAL1(exp2m1);
    TEST_REAL1(log10p1);
    TEST_REAL1(log2p1);
    TEST_REAL1(logp1);
    TEST_REAL1(nextdown);
    TEST_REAL1(nextup);
    TEST_REAL1(roundeven);
    TEST_REAL1(rsqrt);
    TEST_REAL1(sinpi);
    TEST_REAL1(tanpi);
    TEST_REAL1(fabs);

    /* One-argument real-only functions returning an integer type */
    TEST_REAL1_INT(ilogb, int);
    TEST_REAL1_INT(llogb, long);
    TEST_REAL1_INT(lrint, long);
    TEST_REAL1_INT(lround, long);
    TEST_REAL1_INT(llrint, long long);
    TEST_REAL1_INT(llround, long long);

    /* Two-argument real-only functions */
    TEST_REAL2(atan2);
    TEST_REAL2(copysign);
    TEST_REAL2(fdim);
    TEST_REAL2(fmax);
    TEST_REAL2(fmin);
    TEST_REAL2(fmod);
    TEST_REAL2(hypot);
    TEST_REAL2(nextafter);
    TEST_REAL2(remainder);
    TEST_REAL2(atan2pi);
    TEST_REAL2(fmaximum);
    TEST_REAL2(fmaximum_mag);
    TEST_REAL2(fmaximum_num);
    TEST_REAL2(fmaximum_mag_num);
    TEST_REAL2(fminimum);
    TEST_REAL2(fminimum_mag);
    TEST_REAL2(fminimum_num);
    TEST_REAL2(fminimum_mag_num);
    TEST_REAL2(powr);

    /* Complex-only functions returning a real type */
    TEST_COMPLEX_REAL(carg);
    TEST_COMPLEX_REAL(cimag);
    TEST_COMPLEX_REAL(creal);

    /* Complex-only functions returning a complex type */
    TEST_COMPLEX_COMPLEX(conj);
    TEST_COMPLEX_COMPLEX(cproj);
}

static void
test_special(void)
{
    int e;
    int iq;

    /* pow has real and complex versions and two generic arguments */
    CHECK_TYPE(pow(xf, xf), float);
    CHECK_TYPE(pow(xf, xd), double);
    CHECK_TYPE(pow(xi, xf), double);
    CHECK_TYPE(pow(xl, xi), long double);
    CHECK_TYPE(pow(xf, cf), float _Complex);
    CHECK_TYPE(pow(cf, xd), double _Complex);
    CHECK_TYPE(pow(xl, cf), long double _Complex);
    CHECK_TYPE(pow(cd, cl), long double _Complex);
    CHECK_VAL(pow(xf, xf), (powf)(xf, xf));
    CHECK_LVAL(pow(xl, xi), (powl)(xl, xi));
    CHECK_CVAL(pow(xf, cf), (cpowf)(xf, cf));
    CHECK_CVAL(pow(cf, xd), (cpow)(cf, xd));

    /* fabs uses cabs for complex arguments */
    CHECK_TYPE(fabs(cf), float);
    CHECK_TYPE(fabs(cd), double);
    CHECK_TYPE(fabs(cl), long double);
    CHECK_VAL(fabs(3.0f + 4.0f * I), 5.0f);
    CHECK_LVAL(fabs(cl), (cabsl)(cl));

    /* Three generic arguments */
    CHECK_TYPE(fma(xf, xf, xf), float);
    CHECK_TYPE(fma(xf, xf, xd), double);
    CHECK_TYPE(fma(xf, xi, xf), double);
    CHECK_TYPE(fma(xl, xf, xf), long double);
    CHECK_VAL(fma(xf, xf, xf), (fmaf)(xf, xf, xf));
    CHECK_LVAL(fma(xl, xf, xf), (fmal)(xl, xf, xf));

    /* Only some arguments are generic */
    CHECK_TYPE(frexp(xf, &e), float);
    CHECK_TYPE(frexp(xl, &e), long double);
    CHECK_TYPE(ldexp(xf, 3), float);
    CHECK_TYPE(ldexp(xi, 3), double);
    CHECK_TYPE(scalbn(xf, 3), float);
    CHECK_TYPE(scalbln(xl, 3L), long double);
    CHECK_TYPE(nexttoward(xf, xl), float);
    CHECK_TYPE(nexttoward(xd, xl), double);
    CHECK_TYPE(remquo(xf, xf, &iq), float);
    CHECK_TYPE(remquo(xf, xl, &iq), long double);
    CHECK_TYPE(compoundn(xf, 2LL), float);
    CHECK_TYPE(pown(xl, 2LL), long double);
    CHECK_TYPE(rootn(xi, 2LL), double);
    CHECK_TYPE(fromfp(xf, FP_INT_TONEAREST, 32), float);
    CHECK_TYPE(fromfpx(xl, FP_INT_TONEAREST, 32), long double);
    CHECK_TYPE(ufromfp(xd, FP_INT_TONEAREST, 32), double);
    CHECK_TYPE(ufromfpx(xf, FP_INT_TONEAREST, 32), float);
    CHECK_VAL(ldexp(xf, 3), 4.0f);
    CHECK_LVAL(nexttoward(xf, xl), xf);
    CHECK_LVAL(nexttoward(1.0f, 2.0L), (nexttowardf)(1.0f, 2.0L));
    CHECK_LVAL(pown(xl, 2LL), (pownl)(xl, 2LL));

    /*
     * Narrowing functions. The f* macros choose the long double
     * version when any argument is long double. Pick values where the
     * double version would round differently because the extra long
     * double bits are lost when converting to double.
     */
    CHECK_TYPE(fadd(xd, xd), float);
    CHECK_TYPE(fadd(xf, xl), float);
    CHECK_TYPE(fsub(xd, xi), float);
    CHECK_TYPE(fmul(xl, xl), float);
    CHECK_TYPE(fdiv(xf, xf), float);
    CHECK_TYPE(ffma(xd, xd, xl), float);
    CHECK_TYPE(fsqrt(xl), float);
    CHECK_TYPE(dadd(xl, xl), double);
    CHECK_TYPE(dsub(xf, xd), double);
    CHECK_TYPE(dmul(xi, xl), double);
    CHECK_TYPE(ddiv(xl, xd), double);
    CHECK_TYPE(dfma(xl, xl, xl), double);
    CHECK_TYPE(dsqrt(xl), double);
    CHECK_LVAL(dadd(xl, xl), (daddl)(xl, xl));
    CHECK_VAL(fadd(xd, xd), (fadd)(xd, xd));
/*
 * m68k without an FPU emulates long double arithmetic by converting
 * to double, so these values can't show the difference there.
 */
#if defined(HAVE_LONG_DOUBLE_MATH) && LDBL_MANT_DIG > DBL_MANT_DIG + 4 \
    && !(defined(__m68k__) && !defined(__HAVE_68881__))
    {
        /*
         * a needs more precision than double, and b is half an ulp of
         * float at 1, so a + b lies just above a float rounding
         * midpoint, but exactly on it once a is rounded to double.
         */
        volatile long double a = 1.0L + ldexpl(1.0L, -(DBL_MANT_DIG + 4));
        volatile long double b = ldexpl(1.0L, -FLT_MANT_DIG);

        CHECK_VAL(fadd(a, b), (faddl)(a, b));
        if ((faddl)(a, b) == (fadd)((double)a, (double)b)) {
            printf("line %d: faddl and fadd agree, test values are wrong\n", __LINE__);
            errors++;
        }
        CHECK_VAL(ffma(a, 1.0, b), (ffmal)(a, 1.0, b));
    }
#endif

    /* Integer arguments are treated as double */
    CHECK_VAL(sqrt(4), 2.0);
    CHECK_VAL(atan2(xi, xi), (atan2)(2.0, 2.0));
}

/*
 * picolibc declares the long double Bessel functions but doesn't
 * implement them, so only check the result types for long double.
 */
#define TEST_BESSEL1(fn)                     \
    do {                                     \
        CHECK_TYPE(fn(xf), float);           \
        CHECK_TYPE(fn(xd), double);          \
        CHECK_TYPE(fn(xl), long double);     \
        CHECK_TYPE(fn(xi), double);          \
        CHECK_VAL(fn(xf), (fn##f)(xf));      \
        CHECK_VAL(fn(xd), (fn)(xd));         \
        CHECK_VAL(fn(xi), (fn)((double)xi)); \
    } while (0)

/* picolibc extensions to tgmath.h */
static void
test_extensions(void)
{
#ifdef __PICOLIBC__
    int   sg;
    float sf, cf_;

    TEST_BESSEL1(j0);
    TEST_BESSEL1(j1);
    TEST_BESSEL1(y0);
    TEST_BESSEL1(y1);
    TEST_REAL1(gamma);
    TEST_REAL1(significand);
    TEST_REAL1(pow10);
    TEST_REAL2(drem);
    TEST_REAL2(scalb);
    TEST_REAL1_INT(finite, int);

    CHECK_TYPE(jn(2, xf), float);
    CHECK_TYPE(jn(2, xl), long double);
    CHECK_TYPE(yn(2, xi), double);
    CHECK_VAL(jn(2, xf), (jnf)(2, xf));
    CHECK_VAL(yn(2, xd), (yn)(2, xd));

    CHECK_TYPE(lgamma_r(xf, &sg), float);
    CHECK_TYPE(lgamma_r(xi, &sg), double);
    CHECK_TYPE(lgamma_r(xl, &sg), long double);
    CHECK_VAL(lgamma_r(xf, &sg), (lgammaf_r)(xf, &sg));
    CHECK_LVAL(lgamma_r(xl, &sg), (lgammal_r)(xl, &sg));

    sincos(xf, &sf, &cf_);
    CHECK_VAL(sf, (sinf)(xf));
    CHECK_VAL(cf_, (cosf)(xf));

    /* log10 accepts complex arguments as a GNU extension */
    CHECK_TYPE(log10(cf), float _Complex);
    CHECK_TYPE(log10(cl), long double _Complex);
    CHECK_CVAL(log10(cf), (clog10f)(cf));
    CHECK_CVAL(log10(cd), (clog10)(cd));
    CHECK_LCVAL(log10(cl), (clog10l)(cl));
#endif
}

static void
test_single_evaluation(void)
{
    float x[3] = { 0.5f, 0.25f, 0.125f };
    int   i = 0, j = 0, k = 0, m = 0, n = 0;

    (void)sin(x[i++]);
    (void)pow(x[i++], x[j++]);
    (void)fma(x[k++], x[m++], x[n++]);
    (void)fadd(x[j++], 1.0);
    (void)creal(x[j++]);
    if (i != 2 || j != 3 || k != 1 || m != 1 || n != 1) {
        printf("arguments evaluated wrong number of times\n");
        errors++;
    }
}

int
main(void)
{
    test_tables();
    test_sqrt();
    test_special();
    test_extensions();
    test_single_evaluation();
    if (errors)
        printf("%d tgmath errors\n", errors);
    else
        printf("tgmath tests passed\n");
    return errors != 0;
}
