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
#ifdef _HAVE_COMPLEX
#include <complex.h>
#endif

double d1, d2, d3;
float f1, f2, f3;
int i1;
long int li1;
long long int lli1;

#ifdef _TEST_LONG_DOUBLE
long double l1, l2, l3;
#endif

#ifdef _HAVE_COMPLEX
double complex cd1, cd2, cd3;
float complex cf1, cf2, cf3;

#ifdef _TEST_LONG_DOUBLE
long double complex cl1, cl2, cl3;
#endif

#endif

fexcept_t fex;
fenv_t fen;
femode_t fem;

/*
 * Touch test to make sure all of the expected math functions exist
 */

int
main(void)
{
    printf("sizeof float %ld double %ld long double %ld\n",
	   (long) sizeof(float), (long) sizeof(double), (long) sizeof(long double));
    d1 = atan (d1);
    d1 = cos (d1);
    d1 = sin (d1);
    d1 = tan (d1);
    d1 = tanh (d1);
    d1 = frexp (d1, &i1);
    d1 = modf (d1, &d2);
    d1 = ceil (d1);
    d1 = fabs (d1);
    d1 = floor (d1);
    d1 = acos (d1);
    d1 = asin (d1);
    d1 = atan2 (d1, d2);
    d1 = cosh (d1);
    d1 = sinh (d1);
    d1 = exp (d1);
    d1 = ldexp (d1, i1);
    d1 = log (d1);
    d1 = log10 (d1);
    d1 = pow (d1, d2);
    d1 = sqrt (d1);
    d1 = fmod (d1, d2);

    i1 = finite (d1);
    i1 = finitef (f1);
#ifdef _TEST_LONG_DOUBLE
#if defined(_HAVE_BUILTIN_FINITEL) || defined(_HAVE_BUILTIN_ISFINITE)
    i1 = finitel (l1);
#endif
#ifdef _HAVE_BUILTIN_ISINFL
    i1 = isinfl (l1);
#endif
#ifdef _HAVE_BUILTIN_ISNANL
    i1 = isnanl (l1);
#endif
#endif
    i1 = isinff (f1);
    i1 = isnanf (f1);
    i1 = isinf (d1);
    i1 = isnan (d1);

    i1 = __isinff (f1);
    i1 = __isinfd (d1);
    i1 = __isnanf (f1);
    i1 = __isnand (d1);
    i1 = __fpclassifyf (f1);
    i1 = __fpclassifyd (d1);
    i1 = __signbitf (f1);
    i1 = __signbitd (d1);

/* Non ANSI double precision functions.  */

    d1 = infinity ();
    d1 = nan ("");
    d1 = copysign (d1, d2);
    d1 = logb (d1);
    i1 = ilogb (d1);

    d1 = asinh (d1);
    d1 = cbrt (d1);
    d1 = nextafter (d1, d2);
    d1 = rint (d1);
    d1 = scalbn (d1, i1);

    d1 = exp2 (d1);
    d1 = scalbln (d1, li1);
    d1 = tgamma (d1);
    d1 = nearbyint (d1);
    li1 = lrint (d1);
    lli1 = llrint (d1);
    d1 = round (d1);
    li1 = lround (d1);
    lli1 = llround (d1);
    d1 = trunc (d1);
    d1 = remquo (d1, d2, &i1);
    d1 = fdim (d1, d2);
    d1 = fmax (d1, d2);
    d1 = fmin (d1, d2);
    d1 = fma (d1, d2, d3);

    d1 = log1p (d1);
    d1 = expm1 (d1);

    d1 = acosh (d1);
    d1 = atanh (d1);
    d1 = remainder (d1, d2);
    d1 = gamma (d1);
    d1 = lgamma (d1);
    d1 = erf (d1);
    d1 = erfc (d1);
    d1 = log2 (d1);

    d1 = hypot (d1, d2);

/* Single precision versions of ANSI functions.  */

    f1 = atanf (f1);
    f1 = cosf (f1);
    f1 = sinf (f1);
    f1 = tanf (f1);
    f1 = tanhf (f1);
    f1 = frexpf (f1, &i1);
    f1 = modff (f1, &f2);
    f1 = ceilf (f1);
    f1 = fabsf (f1);
    f1 = floorf (f1);

    f1 = acosf (f1);
    f1 = asinf (f1);
    f1 = atan2f (f1, f2);
    f1 = coshf (f1);
    f1 = sinhf (f1);
    f1 = expf (f1);
    f1 = ldexpf (f1, i1);
    f1 = logf (f1);
    f1 = log10f (f1);
    f1 = powf (f1, f2);
    f1 = sqrtf (f1);
    f1 = fmodf (f1, f2);

/* Other single precision functions.  */

    f1 = exp2f (f1);
    f1 = scalblnf (f1, li1);
    f1 = tgammaf (f1);
    f1 = nearbyintf (f1);
    li1 = lrintf (f1);
    lli1 = llrintf (f1);
    f1 = roundf (f1);
    li1 = lroundf (f1);
    lli1 = llroundf (f1);
    f1 = truncf (f1);
    f1 = remquof (f1, f2, &i1);
    f1 = fdimf (f1, f2);
    f1 = fmaxf (f1, f2);
    f1 = fminf (f1, f2);
    f1 = fmaf (f1, f2, f3);

    f1 = infinityf ();
    f1 = nanf ("");
    f1 = copysignf (f1, f2);
    f1 = logbf (f1);
    i1 = ilogbf (f1);

    f1 = asinhf (f1);
    f1 = cbrtf (f1);
    f1 = nextafterf (f1, f2);
    f1 = rintf (f1);
    f1 = scalbnf (f1, i1);
    f1 = log1pf (f1);
    f1 = expm1f (f1);

    f1 = acoshf (f1);
    f1 = atanhf (f1);
    f1 = remainderf (f1, f2);
    f1 = gammaf (f1);
    f1 = lgammaf (f1);
    f1 = erff (f1);
    f1 = erfcf (f1);
    f1 = log2f (f1);
    f1 = hypotf (f1, f2);

#ifdef _TEST_LONG_DOUBLE
    l1 = frexpl (l1, &i1);
    l1 = ldexpl (l1, i1);
    l1 = sqrtl (l1);
    l1 = hypotl (l1, l2);
    l1 = nanl ("");
    i1 = ilogbl (l1);
    l1 = logbl (l1);
    l1 = scalbnl (l1, i1);
    l1 = scalblnl (l1, li1);
    l1 = nearbyintl (l1);
    l1 = rintl (l1);
    li1 = lrintl (l1);
    lli1 = llrintl (l1);
    l1 = roundl (l1);
    l1 = lroundl (l1);
    lli1 = llroundl (l1);
    l1 = truncl (l1);
    l1 = fmaxl (l1, l2);
    l1 = fminl (l1, l2);
    l1 = hypotl (l1, l2);
    l1 = sqrtl (l1);
    l1 = fabsl (l1);
    l1 = copysignl (l1, l2);

    l1 = ceill (l1);
    l1 = floorl (l1);

#ifdef _HAVE_LONG_DOUBLE_MATH
    l1 = atanl (l1);
    l1 = cosl (l1);
    l1 = sinl (l1);
    l1 = tanl (l1);
    l1 = tanhl (l1);
    l1 = modfl (l1, &l2);
    l1 = log1pl (l1);
    l1 = expm1l (l1);

    l1 = acosl (l1);
    l1 = asinl (l1);
    l1 = atan2l (l1, l2);
    l1 = coshl (l1);
    l1 = sinhl (l1);
    l1 = expl (l1);
    l1 = logl (l1);
    l1 = log10l (l1);
    l1 = powl (l1, l2);
    l1 = fmodl (l1, l2);

    l1 = asinhl (l1);
    l1 = cbrtl (l1);
    l1 = log2l (l1);
    l1 = exp2l (l1);
    l1 = tgammal (l1);
    l1 = remquol (l1, l2, &i1);
    l1 = fdiml (l1, l2);
    l1 = fmal (l1, l2, l3);
    l1 = acoshl (l1);
    l1 = atanhl (l1);
    l1 = remainderl (l1, l2);
    l1 = lgammal (l1);
    l1 = erfl (l1);
    l1 = erfcl (l1);
    f1 = dreml (l1, l2);
    sincosl (l1, &l2, &l3);
    l1 = exp10l (l1);
    l1 = pow10l (l1);
    f1 = nexttowardf (f1, l1);
    d1 = nexttoward (d1, l1);
    l1 = nextafterl (l1, l2);
    l1 = nexttowardl (l1, l2);
#endif /* _HAVE_LONG_DOUBLE_MATH */
#endif /* _TEST_LONG_DOUBLE */

    d1 = drem (d1, d2);
    f1 = dremf (f1, f2);
    d1 = lgamma_r (d1, &i1);
    f1 = lgammaf_r (f1, &i1);

    d1 = y0 (d1);
    d1 = y1 (d1);
    d1 = yn (i1, d1);
    d1 = j0 (d1);
    d1 = j1 (d1);
    d1 = jn (i1, d1);

    f1 = y0f (f1);
    f1 = y1f (f1);
    f1 = ynf (i1, f2);
    f1 = j0f (f1);
    f1 = j1f (f1);
    f1 = jnf (i1, f2);

    sincos (d1, &d2, &d3);
    sincosf (f1, &f2, &f3);
    d1 = exp10 (d1);
    d1 = pow10 (d1);
    f1 = exp10f (f1);
    f1 = pow10f (f1);

#ifdef _HAVE_COMPLEX
    cd1 = cacos(cd1);
    cf1 = cacosf(cf1);

/* 7.3.5.2 The casin functions */
    cd1 = casin(cd1);
    cf1 = casinf(cf1);

/* 7.3.5.1 The catan functions */
    cd1 = catan(cd1);
    cf1 = catanf(cf1);

/* 7.3.5.1 The ccos functions */
    cd1 = ccos(cd1);
    cf1 = ccosf(cf1);

/* 7.3.5.1 The csin functions */
    cd1 = csin(cd1);
    cf1 = csinf(cf1);

/* 7.3.5.1 The ctan functions */
    cd1 = ctan(cd1);
    cf1 = ctanf(cf1);

/* 7.3.6 Hyperbolic functions */
/* 7.3.6.1 The cacosh functions */
    cd1 = cacosh(cd1);
    cf1 = cacoshf(cf1);

/* 7.3.6.2 The casinh functions */
    cd1 = casinh(cd1);
    cf1 = casinhf(cf1);

/* 7.3.6.3 The catanh functions */
    cd1 = catanh(cd1);
    cf1 = catanhf(cf1);

/* 7.3.6.4 The ccosh functions */
    cd1 = ccosh(cd1);
    cf1 = ccoshf(cf1);

/* 7.3.6.5 The csinh functions */
    cd1 = csinh(cd1);
    cf1 = csinhf(cf1);

/* 7.3.6.6 The ctanh functions */
    cd1 = ctanh(cd1);
    cf1 = ctanhf(cf1);

/* 7.3.7 Exponential and logarithmic functions */
/* 7.3.7.1 The cexp functions */
    cd1 = cexp(cd1);
    cf1 = cexpf(cf1);

/* 7.3.7.2 The clog functions */
    cd1 = clog(cd1);
    cf1 = clogf(cf1);

/* 7.3.8 Power and absolute-value functions */
/* 7.3.8.1 The cabs functions */
    d1 = cabs(cd1) ;
    f1 = cabsf(cf1) ;

/* 7.3.8.2 The cpow functions */
    cd1 = cpow(cd1, cd2);
    cf1 = cpowf(cf1, cf2);

/* 7.3.8.3 The csqrt functions */
    cd1 = csqrt(cd1);
    cf1 = csqrtf(cf1);

/* 7.3.9 Manipulation functions */
/* 7.3.9.1 The carg functions */ 
    d1 = carg(cd1);
    f1 = cargf(cf1);

/* 7.3.9.2 The cimag functions */
    d1 = cimag(cd1);
    f1 = cimagf(cf1);

/* 7.3.9.3 The conj functions */
    cd1 = conj(cd1);
    cf1 = conjf(cf1);

/* 7.3.9.4 The cproj functions */
    cd1 = cproj(cd1);
    cf1 = cprojf(cf1);

/* 7.3.9.5 The creal functions */
    d1 = creal(cd1);
    f1 = crealf(cf1);

#if __GNU_VISIBLE
    cd1 = clog10(cd1);
    cf1 = clog10f(cf1);
#endif

#ifdef _TEST_LONG_DOUBLE
    cl1 =  csqrtl(cl1);
    l1 = cabsl(cl1) ;
    cl1 = cprojl(cl1);
    l1 = creall(cl1);
    cl1 = conjl(cl1);
    l1 = cimagl(cl1);

#ifdef _HAVE_LONG_DOUBLE_MATH
    l1 = cargl(cl1);
    cl1 = casinl(cl1);
    cl1 = cacosl(cl1);
    cl1 = catanl(cl1);
    cl1 = ccosl(cl1);
    cl1 = csinl(cl1);
    cl1 = ctanl(cl1);
    cl1 = cacoshl(cl1);
    cl1 = casinhl(cl1);
    cl1 = catanhl(cl1);
    cl1 = ccoshl(cl1);
    cl1 = csinhl(cl1);
    cl1 = ctanhl(cl1);
    cl1 = cexpl(cl1);
    cl1 = clogl(cl1);
    cl1 = cpowl(cl1, cl2);
#if __GNU_VISIBLE
    cl1 = clog10l(cl1);
#endif
#endif /* _HAVE_LONG_DOUBLE_MATH */

#endif /* _TEST_LONG_DOUBLE */

#endif /* _HAVE_COMPLEX */

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

    return 0;
}
