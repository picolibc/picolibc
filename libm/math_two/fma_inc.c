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

#include "math_two.h"

#if defined(float_t) && !defined(__HAVE_FAST_FMA_F)

/*
 * Respect the __FLT_EVAL_METHOD__ setting and use the wider type
 */

#if __FLT_EVAL_METHOD__ == 2 && FLOAT_MANT_DIG < LDBL_MANT_DIG

/* Compute using long doubles */
float_t
name(fma)(float_t x, float_t y, float_t z)
{
    return (float_t)fmal((long double)x, (long double)y, (long double)z);
}

#elif __FLT_EVAL_METHOD__ == 1 && FLOAT_MANT_DIG < DBL_MANT_DIG

/* Compute using doubles */
float_t
name(fma)(float_t x, float_t y, float_t z)
{
    return (float_t)fma((double)x, (double)y, (double)z);
}

#else

#if (defined(FE_INEXACT) || defined(FE_UNDERFLOW)) && !defined(__FLOAT_NOEXCEPT_F)
#define HAS_EXCEPTIONS
#endif

#if (defined(FE_UPWARD) || defined(FE_DOWNWARD) || defined(FE_TOWARDZERO)) \
    && !defined(__FLOAT_NOROUND_F)
#define HAS_ROUNDING
#endif

/* Wrap ldexp to avoid trashing errno */
#ifdef __MATH_ERRNO
static float_t
_ldexp_no_errno(float_t x, int n)
{
    int save_errno = errno;
    x = name(ldexp)(x, n);
    errno = save_errno;
    return x;
}
#else
#define _ldexp_no_errno(a, b) name(ldexp)(a, b)
#endif

float_t
name(fma)(float_t x, float_t y, float_t z)
{
    /*
     * Handle special cases. The order of operations and the particular
     * return values here are crucial in handling special cases involving
     * infinities, NaNs, overflows, and signed zeroes correctly.
     */

    if (!isfinite(z) && isfinite(x) && isfinite(y))
        return z + z;
    if (!isfinite(x) || !isfinite(y) || !isfinite(z))
        return (x * y + z);
    if (x == 0 || y == 0)
        return (x * y + z);
    if (z == 0)
        return (x * y);

    float_t xs, ys, zs;
    int     xe, ye, ze;

    xs = name(frexp)(x, &xe);
    ys = name(frexp)(y, &ye);
    zs = name(frexp)(z, &ze);

    int spread = xe + ye - ze;

#ifdef HAS_ROUNDING
    int oround = fegetround();
#define ROUND_NEAREST (oround == FE_TONEAREST)
#else
#define ROUND_NEAREST 1
#endif

    /*
     * If x * y and z are many orders of magnitude apart, the scaling
     * will overflow, so we handle these cases specially.  Rounding
     * modes other than FE_TONEAREST are painful.
     */
    if (spread < -FLOAT_MANT_DIG) {
#ifdef HAS_EXCEPTIONS
        feraiseexcept(FE_INEXACT);
        if (!isnormal(z))
            feraiseexcept(FE_UNDERFLOW);
#endif
#ifdef HAS_ROUNDING
        switch (oround) {
        default:
            break;
#ifdef FE_TOWARDZERO
        case FE_TOWARDZERO:
            if ((x > 0) ^ (y < 0) ^ (z < 0))
                break;
            else
                return name(nextafter)(z, 0);
#endif
#ifdef FE_DOWNWARD
        case FE_DOWNWARD:
            if ((x > 0) ^ (y < 0))
                break;
            else
                return (name(nextafter)(z, -(float_t)INFINITY));
#endif
#ifdef FE_UPWARD
        case FE_UPWARD:
            if ((x > 0) ^ (y < 0))
                return (name(nextafter)(z, (float_t)INFINITY));
            break;
#endif
        }
#endif
        return (z);
    }

#if defined(HAS_EXCEPTIONS) || defined(HAS_ROUNDING)
    /* Save and restore exceptions and rounding modes */
    fenv_t save_env;
    fegetenv(&save_env);
#ifdef HAS_ROUNDING
    fesetround(FE_TONEAREST);
#endif
#define RESTORE fesetenv(&save_env)
#else
#define RESTORE
#endif

    if (spread <= FLOAT_MANT_DIG * 2)
        zs = _ldexp_no_errno(zs, -spread);
    else
        zs = name(copysign)(FLOAT_MIN, zs);

    ff_t    xy = f_mul_f(xs, ys);
    ff_t    rr = f_add_f(xy.hi, zs);

    int     e = xe + ye;

    float_t hi;

    if (rr.hi == 0) {
        RESTORE;
        hi = xy.hi + zs + xy.lo;
    } else {
        hi = rr.hi;

        /* Prepare the low float for use in rounding */
        float_t lo = round_odd_ff(f_add_f(xy.lo, rr.lo));

        if (ROUND_NEAREST) {

            /* compute final exponent value */
            int final_e = name(ilogb)(hi) + e;

            /* Check for denormal result and cope with that */
            if (final_e <= -(FLOAT_MAX_EXP - FLOAT_DENORM_BIAS)) {
                rr = f_add_f(hi, lo);
                hi = rr.hi;
                lo = rr.lo;
                CONST_FORCE_FLOAT_MODIFIER bool inexact = (name(force_eval)(lo) != 0);
                if (inexact) {
                    /*
                     * Compute the number of significand bits which
                     * will be lost in the ldexp call below
                     */
                    int lost_bits = -final_e - (FLOAT_EXP_BIAS - FLOAT_DENORM_BIAS);
                    /*
                     * We want to change the value in two cases:
                     *
                     *  1. We're losing exactly one bit and the
                     *     significand is currently odd. In this case,
                     *     we manually round to even in the direction
                     *     of 'lo'.
                     *
                     *  2. We're losing more than one bit and the significand
                     *     is currently even. In this case, we make its low
                     *     bit summarize the value in 'lo'.
                     */
                    if ((lost_bits == 1) == is_odd_sig(hi))
                        hi = name(nextafter)(hi, (float_t)INFINITY * lo);
                }
                RESTORE;
#ifdef HAS_EXCEPTIONS
                if (inexact)
                    feraiseexcept(FE_INEXACT);
#endif
            } else {
                RESTORE;
                hi = name(force_eval)(hi) + lo;
            }
        } else {
            RESTORE;
            hi = name(force_eval)(hi) + lo;
        }
    }

    hi = _ldexp_no_errno(hi, e);
    return hi;
}

#if defined(WANT_FLOAT32)
_MATH_ALIAS_f_fff(fma)
#elif defined(WANT_FLOAT64)
_MATH_ALIAS_d_ddd(fma)
#endif

#endif

#endif
