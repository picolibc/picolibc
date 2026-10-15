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

float_t
name(rootn)(float_t x, long long int y)
{
    /*
     * Check for invalid operands:
     *
     * y zero.
     * x negative and y even
     */
    if (y == 0 || (isless(x, 0) && (y & 1) == 0))
        return kernel(__math_invalid)(0);

    /* Check for nan x */
    if (isnan(x))
        return x + x;

    /*
     * Infinite x
     *
     * y > 0: x
     * y < 0: 1/x
     */
    if (isinf(x)) {
        if (y > 0)
            return x;
        return 1 / x;
    }

    /*
     * x == 0
     *
     * y < 0 && y odd && negative x: -infinity
     * y < 0 && (y even || positive x): infinity
     * y > 0 && y even: 0
     * y > 0 && y odd: x (preserve sign)
     */
    if (x == 0) {
        if (y < 0)
            return kernel(__math_divzero)((y & 1) != 0 && signbit(x) != 0);
        if (y & 1)
            return x;
        return 0;
    }

    /*
     * y == 1
     */
    if (y == 1)
        return x;

    /*
     * y == -1
     *
     * Could overflow
     */
    if (y == -1)
        return kernel(__math_check_oflow)(1 / x);

    float_t ret;
    float_t abs_x = name(fabs)(x);

    if (y >= 4 * FLOAT_MAX_EXP || y <= -4 * FLOAT_MAX_EXP) {
        ret = name(pow)(abs_x, float_f(1.0) / y);
    } else {
        ff_t    quo = ff_div_f(f_to_ff(1), y);
        float_t ret_h = name(pow)(abs_x, quo.hi);
        float_t ret_l = name(pow)(abs_x, quo.lo);
        ret = ret_h * ret_l;
    }
    return name(copysign)(ret, x);
}

#if defined(WANT_FLOAT32)
_MATH_ALIAS_f_fk(rootn)
#elif defined(WANT_FLOAT64)
_MATH_ALIAS_d_dk(rootn)
#endif

#endif
