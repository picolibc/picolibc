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

#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

double d1, d2, d3;
float f1, f2, f3;
int i1;
long int li1;
long long int lli1;

#ifdef _HAVE_LONG_DOUBLE
long double l1, l2, l3;
#endif

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
#if defined(_HAVE_BUILTIN_FINITEL) || defined(_HAVE_BUILTIN_ISFINITE)
    i1 = finitel (l1);
#endif
    i1 = isinff (f1);
    i1 = isnanf (f1);
#ifdef _HAVE_BUILTIN_ISINFL
    i1 = isinfl (l1);
#endif
#ifdef _HAVE_BUILTIN_ISNANL
    i1 = isnanl (l1);
#endif
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

#ifdef _LDBL_EQ_DBL
    l1 = atanl (l1);
    l1 = cosl (l1);
    l1 = sinl (l1);
    l1 = tanl (l1);
    l1 = tanhl (l1);
    l1 = frexpl (l1, &i1);
    l1 = modfl (l1, &l2);
    l1 = ceill (l1);
    l1 = floorl (l1);
    l1 = log1pl (l1);
    l1 = expm1l (l1);
#endif

#ifdef _LDBL_EQ_DBL
    l1 = acosl (l1);
    l1 = asinl (l1);
    l1 = atan2l (l1, l2);
    l1 = coshl (l1);
    l1 = sinhl (l1);
    l1 = expl (l1);
    l1 = ldexpl (l1, i1);
    l1 = logl (l1);
    l1 = log10l (l1);
    l1 = powl (l1, l2);
#endif
    l1 = sqrtl (l1);
#ifdef _LDBL_EQ_DBL
    l1 = fmodl (l1, l2);
#endif
    l1 = hypotl (l1, l2);

#ifdef _LDBL_EQ_DBL
    l1 = nanl ("");
    i1 = ilogbl (l1);
    l1 = asinhl (l1);
    l1 = cbrtl (l1);
    l1 = nextafterl (l1, l2);
    f1 = nexttowardf (f1, l1);
    d1 = nexttoward (d1, l1);
    l1 = nexttowardl (l1, l2);
    l1 = logbl (l1);
    l1 = log2l (l1);
    l1 = rintl (l1);
    l1 = scalbnl (l1, i1);
    l1 = exp2l (l1);
    l1 = scalblnl (l1, li1);
    l1 = tgammal (l1);
    l1 = nearbyintl (l1);
    li1 = lrintl (l1);
    lli1 = llrintl (l1);
    l1 = roundl (l1);
    l1 = lroundl (l1);
    lli1 = llroundl (l1);
    l1 = truncl (l1);
    l1 = remquol (l1, l2, &i1);
    l1 = fdiml (l1, l2);
    l1 = fmaxl (l1, l2);
    l1 = fminl (l1, l2);
    l1 = fmal (l1, l2, l3);
#endif

#ifdef _LDBL_EQ_DBL
    l1 = acoshl (l1);
    l1 = atanhl (l1);
    l1 = remainderl (l1, l2);
    l1 = lgammal (l1);
    l1 = erfl (l1);
    l1 = erfcl (l1);
#endif

    l1 = hypotl (l1, l2);
    l1 = sqrtl (l1);

#ifdef _LDBL_EQ_DBL
    l1 = rintl (l1);
    li1 = lrintl (l1);
    lli1 = llrintl (l1);
#endif

#ifdef _LDBL_EQ_DBL
    l1 = fabsl (l1);
    l1 = copysignl (l1, l2);
#endif

    d1 = drem (d1, d2);
    f1 = dremf (f1, f2);
#ifdef _LDBL_EQ_DBL
    f1 = dreml (l1, l2);
#endif
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
#ifdef _LDBL_EQ_DBL
    sincosl (l1, &l2, &l3);
#endif
    d1 = exp10 (d1);
    d1 = pow10 (d1);
    f1 = exp10f (f1);
    f1 = pow10f (f1);
#ifdef _LDBL_EQ_DBL
    l1 = exp10l (l1);
    l1 = pow10l (l1);
#endif

    return 0;
}
