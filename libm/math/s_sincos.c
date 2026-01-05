/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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
#include "fdlibm.h"
#include <errno.h>
#include <math.h>

#ifdef _NEED_FLOAT64

void __no_builtin
sincos64(__float64 x, __float64 *sinx, __float64 *cosx)
{
    int32_t ix;

    /* High word of x. */
    GET_HIGH_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3fe921fb) {
        *sinx = __kernel_sin(x, _F_64(0.0), 0);
        *cosx = __kernel_cos(x, _F_64(0.0));
        return;
    }

    /* sin(Inf or NaN) is NaN */
    else if (ix >= 0x7ff00000) {
        *sinx = *cosx = __math_invalid(x);
        return;
    }

    /* argument reduction needed */
    else {
        __float64 y[2];
        int32_t   n = __rem_pio2(x, y);
        __float64 s = __kernel_sin(y[0], y[1], 1);
        __float64 c = __kernel_cos(y[0], y[1]);

        switch (n & 3) {
        case 0:
            *sinx = s;
            *cosx = c;
            break;
        case 1:
            *sinx = c;
            *cosx = -s;
            break;
        case 2:
            *sinx = -s;
            *cosx = -c;
            break;
        default:
            *sinx = -c;
            *cosx = s;
            break;
        }
    }
}

_MATH_ALIAS_v_dDD(sincos)

#endif /* _NEED_FLOAT64 */
