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
#include "math_two.h"

#ifdef float_t

static const ff_t    _log_n = { .hi = LOG_N_HI, .lo = LOG_N_LO };
static const float_t _limit = -FLOAT_MAX / LOG_N_HI;

float_t
name(FUNC)(float_t x)
{
    /*
     * Compute N ** x - 1:
     *
     * p = log(N) * x
     * r_hi = expm1(p.hi) + 1
     * r_lo = expm1(p.lo) + 1
     * r = r_hi * r_lo
     * m = (r - 1)
     */

    if (!isfinite(x)) {
        if (isnan(x))
            return x + x;
        if (x < 0)
            return -1;
        return x;
    }
    if (x < _limit)
        return kernel(__math_inexact)(FLOAT_DENORM_MIN - 1);

    ff_t p = ff_mul_f(_log_n, x);

    /*
     * Check for overflow in the multiply, which is only possible when
     * N > e (or log(N) > 1)
     */
    if (LOG_N_HI > 1 && isinf(p.hi))
        return kernel(__math_oflow)(0);
    float_t hi = name(expm1)(p.hi);
    if (!isfinite(hi) || hi == -1)
        return hi;
    float_t lo = name(expm1)(p.lo);
    ff_t    r_hi = f_add_f(hi, 1);
    ff_t    r_lo = f_add_f(lo, 1);
    ff_t    r = ff_mul_ff(r_hi, r_lo);
    ff_t    m = ff_add_f(r, -1);

    return m.hi;
}

#endif
