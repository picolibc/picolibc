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

#include "fma_common_inc.c"

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

#ifdef HAS_ROUNDING
    int oround = fegetround();
#define ROUND_NEAREST (oround == FE_TONEAREST)
#else
#define ROUND_NEAREST 1
#endif

    /* Save and restore exceptions and rounding modes */
    SAVE_ENV;

    int                             e;
    ff_t                            rr = name(_fma_scaled)(x, y, z, &e);
    float_t                         hi;
    CONST_FORCE_FLOAT_MODIFIER bool inexact = false;

    if (rr.hi == 0) {
        RESTORE;
        /* An exact zero takes its sign from the rounding mode */
        if (rr.lo == 0)
            return x * y + z;
        hi = rr.lo;
    } else {
        hi = rr.hi;
        float_t lo = rr.lo;
        inexact = (lo != 0);

        if (ROUND_NEAREST) {

            /* compute final exponent value */
            int final_e = name(ilogb)(hi) + e;

            /* Check for denormal result and cope with that */
            if (final_e <= -(FLOAT_MAX_EXP - FLOAT_DENORM_BIAS)) {
                rr = f_add_f(hi, lo);
                hi = rr.hi;
                lo = rr.lo;
                inexact = (lo != 0);
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
#ifdef HAS_EXCEPTIONS
    /* An inexact result smaller than the smallest normal has underflowed */
    if (inexact && name(fabs)(hi) < FLOAT_MIN)
        feraiseexcept(FE_UNDERFLOW);
#endif
    return hi;
}

#if defined(WANT_FLOAT32)
_MATH_ALIAS_f_fff(fma)
#elif defined(WANT_FLOAT64)
_MATH_ALIAS_d_ddd(fma)
#endif

#endif

#endif
