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
#if __OBSOLETE_MATH_FLOAT

#include <errno.h>

void __no_builtin
sincosf(float x, float *sinx, float *cosx)
{
    int32_t ix;

    GET_FLOAT_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3f490fd8) {
        *sinx = __kernel_sinf(x, 0.0f, 0);
        *cosx = __kernel_cosf(x, 0.0f);
        return;
    }

    /* sin(Inf or NaN) is NaN */
    else if (!FLT_UWORD_IS_FINITE(ix)) {
        *sinx = *cosx = __math_invalidf(x);
        return;
    }

    /* argument reduction needed */
    else {
        float y[2];
        int   n = __rem_pio2f(x, y);
        float s = __kernel_sinf(y[0], y[1], 1);
        float c = __kernel_cosf(y[0], y[1]);

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

_MATH_ALIAS_v_fFF(sincos)

#else
#include "../common/sincosf.c"
#endif /* __OBSOLETE_MATH_FLOAT */
