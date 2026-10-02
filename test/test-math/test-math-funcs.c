/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#define __STDC_WANT_IEC_60559_BFP_EXT__
#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <fenv.h>
#include <stdlib.h>
#ifdef __HAVE_COMPLEX
#include <complex.h>
#endif

volatile double        d1, d2, d3;
volatile float         f1, f2, f3;
volatile int           i1;
volatile unsigned int  u1;
volatile long int      li1;
volatile long long int lli1;

#ifdef _TEST_LONG_DOUBLE
volatile long double l1, l2, l3;
#endif

#ifdef __HAVE_COMPLEX
volatile double complex cd1, cd2, cd3;
volatile float complex  cf1, cf2, cf3;

#ifdef _TEST_LONG_DOUBLE
volatile long double complex cl1, cl2, cl3;
#endif

#endif

fexcept_t fex;
fenv_t    fen;
femode_t  fem;

/*
 * Touch test to make sure all of the expected math functions exist
 */

void      test_trig(void);
void
test_trig(void)
{
    d1 = cos(d1);
    d1 = sin(d1);
    d1 = tan(d1);
    sincos(d1, (double *)&d2, (double *)&d3);

    d1 = acos(d1);
    d1 = asin(d1);
    d1 = atan(d1);
    d1 = atan2(d1, d2);

    d1 = cospi(d1);
    d1 = sinpi(d1);
    d1 = tanpi(d1);

    d1 = acospi(d1);
    d1 = asinpi(d1);
    d1 = atanpi(d1);
    d1 = atan2pi(d1, d2);

    d1 = cosh(d1);
    d1 = sinh(d1);
    d1 = tanh(d1);

    d1 = acosh(d1);
    d1 = asinh(d1);
    d1 = atanh(d1);

    f1 = cosf(f1);
    f1 = sinf(f1);
    f1 = tanf(f1);
    sincosf(f1, (float *)&f2, (float *)&f3);

    f1 = cospif(f1);
    f1 = sinpif(f1);
    f1 = tanpif(f1);

    f1 = acosf(f1);
    f1 = asinf(f1);
    f1 = atanf(f1);
    f1 = atan2f(f1, f2);

    f1 = acospif(f1);
    f1 = asinpif(f1);
    f1 = atanpif(f1);
    f1 = atan2pif(f1, f2);

    f1 = coshf(f1);
    f1 = sinhf(f1);
    f1 = tanhf(f1);

    f1 = acoshf(f1);
    f1 = asinhf(f1);
    f1 = atanhf(f1);

#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
    l1 = cosl(l1);
    l1 = sinl(l1);
    l1 = tanl(l1);
    sincosl(l1, (long double *)&l2, (long double *)&l3);

    l1 = cospil(l1);
    l1 = sinpil(l1);
    l1 = tanpil(l1);

    l1 = acosl(l1);
    l1 = asinl(l1);
    l1 = atanl(l1);
    l1 = atan2l(l1, l2);

    l1 = acospil(l1);
    l1 = asinpil(l1);
    l1 = atanpil(l1);
    l1 = atan2pil(l1, l2);

    l1 = coshl(l1);
    l1 = sinhl(l1);
    l1 = tanhl(l1);

    l1 = acoshl(l1);
    l1 = asinhl(l1);
    l1 = atanhl(l1);

#endif /* _TEST_LONG_DOUBLE */
}

void test_exp(void);
void
test_exp(void)
{
    d1 = exp(d1);
    d1 = log(d1);
    d1 = log10(d1);
    d1 = pow(d1, d2);
    d1 = logb(d1);
    i1 = ilogb(d1);
    d1 = exp2(d1);
    d1 = log1p(d1);
    d1 = expm1(d1);
    d1 = log2(d1);
    d1 = exp10(d1);
    d1 = pown(d1, li1);
    d1 = exp10m1(d1);
    d1 = exp2m1(d1);
    li1 = llogb(d1);
    d1 = log10p1(d1);
    d1 = log2p1(d1);
    d1 = logp1(d1);
    d1 = powr(d1, d2);

#ifdef __PICOLIBC__
    d1 = pow10(d1);
#endif

#ifndef __PICOLIBC__
    d1 = rootn(d1, li1);
#endif

    f1 = expf(f1);
    f1 = logf(f1);
    f1 = log10f(f1);
    f1 = powf(f1, f2);
    f1 = logbf(f1);
    i1 = ilogbf(f1);
    f1 = exp2f(f1);
    f1 = log1pf(f1);
    f1 = expm1f(f1);
    f1 = log2f(f1);
    f1 = exp10f(f1);
    f1 = pownf(f1, li1);
    f1 = exp10m1f(f1);
    f1 = exp2m1f(f1);
    li1 = llogbf(d1);
    f1 = log10p1f(f1);
    f1 = log2p1f(f1);
    f1 = logp1f(f1);
    f1 = powrf(f1, f2);

#ifdef __PICOLIBC__
    f1 = pow10f(f1);
#endif

#ifndef __PICOLIBC__
    f1 = rootnf(f1, li1);
#endif

#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
    i1 = ilogbl(l1);
    l1 = logbl(l1);
    l1 = log1pl(l1);
    l1 = expm1l(l1);

    l1 = expl(l1);
    l1 = logl(l1);
    l1 = log10l(l1);
    l1 = powl(l1, l2);
    l1 = log2l(l1);
    l1 = exp2l(l1);
    l1 = exp10l(l1);
    l1 = pownl(l1, li1);
    l1 = exp10m1l(l1);
    l1 = exp2m1l(l1);

    li1 = llogbl(l1);
    l1 = log10p1l(l1);
    l1 = log2p1l(l1);
    l1 = logp1l(l1);
    l1 = powrl(l1, l2);

#ifdef __PICOLIBC__
    l1 = pow10l(l1);
#endif

#ifndef __PICOLIBC__
    l1 = rootnl(l1, li1);
#endif

#endif /* _TEST_LONG_DOUBLE */
}

void test_bits(void);
void
test_bits(void)
{
    d1 = frexp(d1, (int *)&i1);
    d1 = modf(d1, (double *)&d2);
    d1 = ceil(d1);
    d1 = fabs(d1);
    d1 = floor(d1);
    d1 = ldexp(d1, i1);
    d1 = fmod(d1, d2);

    d1 = nextafter(d1, d2);
    d1 = nextup(d1);
    d1 = nextdown(d1);
    d1 = rint(d1);
    d1 = scalbn(d1, i1);
    d1 = scalb(d1, d2);
    d1 = copysign(d1, d2);
    d1 = scalbln(d1, li1);
    i1 = canonicalize((double *)&d1, (double *)&d2);
    d1 = compoundn(d1, lli1);

    d1 = nearbyint(d1);
    li1 = lrint(d1);
    lli1 = llrint(d1);
    d1 = round(d1);
    li1 = lround(d1);
    lli1 = llround(d1);
    d1 = trunc(d1);
    d1 = remquo(d1, d2, (int *)&i1);
    d1 = remainder(d1, d2);
    d1 = drem(d1, d2);
    d1 = getpayload((double *)&d1);
    d1 = roundeven(d1);

#ifndef __PICOLIBC__
    d1 = fromfp(d1, i1, u1);
    d1 = fromfpx(d1, i1, u1);
    d1 = ufromfp(d1, i1, u1);
    d1 = ufromfpx(d1, i1, u1);
#endif

#ifdef __PICOLIBC__
    d1 = infinity();
#endif
    d1 = nan("");

    f1 = ldexpf(f1, i1);
    f1 = fmodf(f1, f2);
    f1 = frexpf(f1, (int *)&i1);
    f1 = modff(f1, (float *)&f2);
    f1 = ceilf(f1);
    f1 = fabsf(f1);
    f1 = floorf(f1);
    f1 = scalblnf(f1, li1);
    f1 = nearbyintf(f1);
    li1 = lrintf(f1);
    lli1 = llrintf(f1);
    f1 = roundf(f1);
    li1 = lroundf(f1);
    lli1 = llroundf(f1);
    f1 = truncf(f1);
    f1 = remquof(f1, f2, (int *)&i1);
    f1 = remainderf(f1, f2);
    f1 = dremf(f1, f2);
    f1 = getpayloadf((float *)&f1);

    f1 = copysignf(f1, f2);
    f1 = nextafterf(f1, f2);
    f1 = nextdownf(f1);
    f1 = nextupf(f1);
    f1 = rintf(f1);
    f1 = scalbnf(f1, i1);
    f1 = scalbf(f1, f2);
    f1 = compoundnf(f1, lli1);
    i1 = canonicalizef((float *)&f1, (float *)&f2);
    f1 = roundevenf(f1);

#ifndef __PICOLIBC__
    f1 = fromfpf(f1, i1, u1);
    f1 = fromfpxf(f1, i1, u1);
    f1 = ufromfpf(f1, i1, u1);
    f1 = ufromfpxf(f1, i1, u1);
#endif

#ifdef __PICOLIBC__
    f1 = infinityf();
#endif
    f1 = nanf("");

#ifdef _TEST_LONG_DOUBLE
    l1 = scalbnl(l1, i1);
    l1 = scalbl(l1, l2);
    l1 = scalblnl(l1, li1);
    l1 = nearbyintl(l1);
    l1 = rintl(l1);
    li1 = lrintl(l1);
    lli1 = llrintl(l1);
    l1 = roundl(l1);
    l1 = lroundl(l1);
    lli1 = llroundl(l1);
    l1 = truncl(l1);
    l1 = fabsl(l1);
    l1 = copysignl(l1, l2);
    l1 = ceill(l1);
    l1 = floorl(l1);

#ifdef __HAVE_LONG_DOUBLE_MATH
    l1 = modfl(l1, (long double *)&l2);
    l1 = fmodl(l1, l2);
    l1 = remquol(l1, l2, (int *)&i1);
    l1 = remainderl(l1, l2);
    f1 = dreml(l1, l2);
    l1 = getpayloadl((long double *)&l1);

    l1 = nextafterl(l1, l2);
    l1 = nextdownl(l1);
    l1 = nextupl(l1);

    f1 = nexttowardf(f1, l1);
    d1 = nexttoward(d1, l1);
    l1 = nexttowardl(l1, l2);

    i1 = canonicalizel((long double *)&l1, (long double *)&l2);
    l1 = compoundnl(l1, lli1);
    l1 = roundevenl(l1);
#endif

#ifdef __PICOLIBC__
    l1 = infinityl();
#endif
    l1 = nanl("");

#ifndef __PICOLIBC__
    l1 = fromfpl(l1, i1, u1);
    l1 = fromfpxl(l1, i1, u1);

    l1 = ufromfpl(l1, i1, u1);
    l1 = ufromfpxl(l1, i1, u1);
#endif
#endif /* _TEST_LONG_DOUBLE */
}

void test_tests(void);
void
test_tests(void)
{
    i1 = finite(d1);
    i1 = isinf(d1);
    i1 = isnan(d1);

    i1 = iscanonical(d1);

    i1 = finitef(f1);
    i1 = isinff(f1);
    i1 = isnanf(f1);

    i1 = iscanonical(f1);

#ifdef __PICOLIBC__
    i1 = __isinff(f1);
    i1 = __isinfd(d1);
    i1 = __isnanf(f1);
    i1 = __isnand(d1);
    i1 = __fpclassifyf(f1);
    i1 = __fpclassifyd(d1);
    i1 = __signbitf(f1);
    i1 = __signbitd(d1);
#endif

#ifdef _TEST_LONG_DOUBLE
    i1 = finitel(l1);
    i1 = isinfl(l1);
    i1 = isnanl(l1);

    i1 = iscanonical(l1);
#endif /* _TEST_LONG_DOUBLE */
}

void test_other(void);
void
test_other(void)
{
    d1 = sqrt(d1);
    d1 = cbrt(d1);
    d1 = fma(d1, d2, d3);
    d1 = tgamma(d1);
    d1 = gamma(d1);
    d1 = lgamma(d1);
    d1 = lgamma_r(d1, (int *)&i1);
    d1 = erf(d1);
    d1 = erfc(d1);
    d1 = hypot(d1, d2);

    d1 = y0(d1);
    d1 = y1(d1);
    d1 = yn(i1, d1);
    d1 = j0(d1);
    d1 = j1(d1);
    d1 = jn(i1, d1);

#ifndef __PICOLIBC__
    d1 = rsqrt(d1);
#endif

    f1 = cbrtf(f1);
    f1 = sqrtf(f1);
    f1 = fmaf(f1, f2, f3);
    f1 = tgammaf(f1);
    f1 = lgammaf_r(f1, (int *)&i1);
    f1 = gammaf(f1);
    f1 = lgammaf(f1);
    f1 = erff(f1);
    f1 = erfcf(f1);
    f1 = hypotf(f1, f2);

    f1 = y0f(f1);
    f1 = y1f(f1);
    f1 = ynf(i1, f2);
    f1 = j0f(f1);
    f1 = j1f(f1);
    f1 = jnf(i1, f2);

#ifndef __PICOLIBC__
    f1 = rsqrtf(f1);
#endif

#ifdef _TEST_LONG_DOUBLE
    l1 = frexpl(l1, (int *)&i1);
    l1 = ldexpl(l1, i1);
    l1 = sqrtl(l1);
    l1 = hypotl(l1, l2);
#ifdef __HAVE_LONG_DOUBLE_MATH
    l1 = cbrtl(l1);
    l1 = tgammal(l1);
    l1 = gammal(l1);
    l1 = fmal(l1, l2, l3);
    l1 = lgammal(l1);
    l1 = lgammal_r(l1, (int *)&i1);
    l1 = erfl(l1);
    l1 = erfcl(l1);
#ifndef __PICOLIBC__
    l1 = rsqrtl(l1);
#endif
#endif
#endif
}

void test_minmaxl(void);

void
test_minmaxl(void)
{
#ifdef _TEST_LONG_DOUBLE
    l1 = fmaxl(l1, l2);
    l1 = fminl(l1, l2);

#ifdef __HAVE_LONG_DOUBLE_MATH
    l1 = fdiml(l1, l2);
#ifndef __clang__
    /* ??? clang 21 on x86_64 crashes if these lines are included */
    l1 = fmaximuml(l1, l2);
    l1 = fmaximum_magl(l1, l2);
    l1 = fmaximum_mag_numl(l1, l2);
    l2 = fmaximum_numl(l1, l2);

    l1 = fminimuml(l1, l2);
    l1 = fminimum_magl(l1, l2);
    l1 = fminimum_mag_numl(l1, l2);
    l1 = fminimum_numl(l1, l2);
#endif
#endif
#endif
}

void test_minmax(void);

void
test_minmax(void)
{
    d1 = fdim(d1, d2);
    d1 = fmax(d1, d2);
    d1 = fmin(d1, d2);

    d1 = fmaximum(d1, d2);
    d1 = fmaximum_mag(d1, d2);
    d1 = fmaximum_mag_num(d1, d2);
    d1 = fmaximum_num(d1, d2);

    d1 = fminimum(d1, d2);
    d1 = fminimum_mag(d1, d2);
    d1 = fminimum_mag_num(d1, d2);
    d1 = fminimum_num(d1, d2);
    d1 = fminimum(d1, d2);
    d1 = fminimum_mag(d1, d2);
    d1 = fminimum_mag_num(d1, d2);
    d1 = fminimum_num(d1, d2);

    f1 = fdimf(f1, f2);
    f1 = fmaxf(f1, f2);
    f1 = fminf(f1, f2);

    f1 = fmaximumf(f1, f2);
    f1 = fmaximum_magf(f1, f2);
    f1 = fmaximum_mag_numf(f1, f2);
    f1 = fmaximum_numf(f1, f2);
    f1 = fminimumf(f1, f2);
    f1 = fminimum_magf(f1, f2);
    f1 = fminimum_mag_numf(f1, f2);
    f1 = fminimum_numf(f1, f2);

    test_minmaxl();
}

void test_narrow(void);
void
test_narrow(void)
{
    f1 = fadd(d1, d2);
    f1 = fdiv(d1, d2);
    f1 = ffma(d1, d2, d3);
    f1 = fmul(d1, d2);
    f1 = fsqrt(d1);
    f1 = fsub(d1, d2);

#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
    f1 = faddl(l1, l2);
    f1 = fdivl(l1, l2);
    f1 = ffmal(l1, l2, l3);
    f1 = fmull(l1, l2);
    f1 = fsqrtl(l1);
    f1 = fsubl(l1, l2);

    d1 = daddl(l1, l2);
    d1 = ddivl(l1, l2);
    d1 = dfmal(l1, l2, l3);
    d1 = dmull(l1, l2);
    d1 = dsqrtl(l1);
    d1 = dsubl(l1, l2);
#endif
}

void test_complex_trig(void);
void
test_complex_trig(void)
{
#ifdef __HAVE_COMPLEX
    cd1 = cacos(cd1);
    cd1 = casin(cd1);
    cd1 = catan(cd1);

    cd1 = ccos(cd1);
    cd1 = csin(cd1);
    cd1 = ctan(cd1);

    cd1 = ccosh(cd1);
    cd1 = csinh(cd1);
    cd1 = ctanh(cd1);

    cd1 = cacosh(cd1);
    cd1 = casinh(cd1);
    cd1 = catanh(cd1);

    cf1 = ccosf(cf1);
    cf1 = csinf(cf1);
    cf1 = ctanf(cf1);

    cf1 = cacosf(cf1);
    cf1 = casinf(cf1);
    cf1 = catanf(cf1);

    cf1 = cacoshf(cf1);
    cf1 = casinhf(cf1);
    cf1 = catanhf(cf1);

    cf1 = ccoshf(cf1);
    cf1 = csinhf(cf1);
    cf1 = ctanhf(cf1);
#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
    cl1 = ccosl(cl1);
    cl1 = csinl(cl1);
    cl1 = ctanl(cl1);

    cl1 = casinl(cl1);
    cl1 = cacosl(cl1);
    cl1 = catanl(cl1);

    cl1 = ccoshl(cl1);
    cl1 = csinhl(cl1);
    cl1 = ctanhl(cl1);

    cl1 = cacoshl(cl1);
    cl1 = casinhl(cl1);
    cl1 = catanhl(cl1);
#endif
#endif
}

void test_complex_exp(void);

void
test_complex_exp(void)
{
#ifdef __HAVE_COMPLEX
    /* 7.3.7 Exponential and logarithmic functions */
    /* 7.3.7.1 The cexp functions */
    cd1 = cexp(cd1);
    cf1 = cexpf(cf1);

    /* 7.3.7.2 The clog functions */
    cd1 = clog(cd1);
    cf1 = clogf(cf1);

    cd1 = cpow(cd1, cd2);
    cf1 = cpowf(cf1, cf2);

    cd1 = clog10(cd1);
    cf1 = clog10f(cf1);

#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
    cl1 = cexpl(cl1);
    cl1 = clogl(cl1);
    cl1 = cpowl(cl1, cl2);
    cl1 = clog10l(cl1);
#endif
#endif
}

void test_complex_other(void);

void
test_complex_other(void)
{
#ifdef __HAVE_COMPLEX
    cd1 = csqrt(cd1);

    cf1 = csqrtf(cf1);

#ifdef _TEST_LONG_DOUBLE
    cl1 = csqrtl(cl1);
#endif
#endif
}

void test_complex_bits(void);

void
test_complex_bits(void)
{
#ifdef __HAVE_COMPLEX
    d1 = cabs(cd1);
    d1 = carg(cd1);
    d1 = cimag(cd1);
    d1 = creal(cd1);
    cd1 = conj(cd1);
    cd1 = cproj(cd1);

    f1 = cabsf(cf1);
    f1 = cargf(cf1);
    f1 = cimagf(cf1);
    f1 = crealf(cf1);
    cf1 = conjf(cf1);
    cf1 = cprojf(cf1);

#ifdef _TEST_LONG_DOUBLE
    l1 = cabsl(cl1);
    cl1 = cprojl(cl1);
    l1 = creall(cl1);
    cl1 = conjl(cl1);
    l1 = cimagl(cl1);
#ifdef __HAVE_LONG_DOUBLE_MATH
    l1 = cargl(cl1);
#endif
#endif
#endif
}

void test_except(void);

void
test_except(void)
{
    i1 = feclearexcept(FE_ALL_EXCEPT);
    i1 = fegetexceptflag(&fex, FE_ALL_EXCEPT);
    i1 = feraiseexcept(0);
    i1 = fesetexceptflag(&fex, FE_ALL_EXCEPT);
    i1 = fetestexcept(FE_ALL_EXCEPT);

    i1 = fegetround();
    i1 = fesetround(FE_TONEAREST);

    i1 = fegetenv(&fen);
    i1 = feholdexcept(&fen);
    i1 = fesetenv(&fen);
    i1 = feupdateenv(&fen);

    i1 = feenableexcept(FE_ALL_EXCEPT);
    i1 = fedisableexcept(FE_ALL_EXCEPT);
    i1 = fegetexcept();

    i1 = fegetmode(&fem);
    i1 = fesetmode(&fem);
    i1 = fesetexcept(FE_ALL_EXCEPT);
}

int
main(void)
{
    printf("sizeof float %ld double %ld long double %ld\n", (long)sizeof(float),
           (long)sizeof(double), (long)sizeof(long double));

    test_trig();
    test_exp();
    test_bits();
    test_tests();
    test_other();
    test_minmax();

    test_complex_trig();
    test_complex_exp();
    test_complex_other();
    test_complex_bits();

    test_except();

    return 0;
}
