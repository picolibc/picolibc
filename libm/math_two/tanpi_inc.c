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

#include "trigpi.h"

#ifdef float_t

float_t
name(tanpi)(float_t x)
{
    ff_t      prod;
    __int32_t ix;
    int       n;

    GET_EXP(ix, x);

    if (EXP_IS_NONFINITE(ix))
        return kernel(__math_invalid)(x);

    /* check for integers and return zero immediately */
    if (_isint_float_f(x))
        return float_f(0.0);

    /* pin .5 values to +infinity */
    if (_isint_float_f(x * 2) == 1)
        return (float_t)INFINITY;

    /* |x| < 0.25 */
    if (ix < EXP_ONE_QUARTER) {
        n = 0;
    } else {
        n = name(_rem_half)(x, &x);
    }

    prod = ff_mul_f(name(_pi_ff_), x);
#if defined(WANT_FLOAT80) || defined(WANT_FLOAT128)
    n = n & 1;
#else
    n = 1 - ((n & 1) << 1);
#endif
    return kernel(__kernel_tan)(prod.hi, prod.lo, n);
}

#endif
