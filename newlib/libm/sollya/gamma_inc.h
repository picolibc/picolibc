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

#include "math2.h"

#ifdef float_t
static const float_t gamma_coeffs[] = COEFFS;

float_t
name(gamma)(float_t x)
{
    /* non-finite values */
    if (!isfinite(x))
        return x + x;

    /* non-positive integers return INF */
    if (x <= 0 && floor_f(x) == x)
        return (float_t) INFINITY;

    /* Large positive values overflow */
    if (x >= MAX_ARG)
        return (float_t) INFINITY;

    float_t     p = float_f(0.5);
    float_t     eps = 0;
    ff_t        ff;

#define mix_in(v) do {                          \
        ff = two_mul(p, (v));                   \
        p = ff.hi;                              \
        eps += ff.lo / p;                       \
    } while(0)

    if (x < 1) {
        float_t fl = floor_f(x);
        mix_in(1/(x - fl));
        if (x < 0) {
            mix_in(1/(x - (fl+1)));
            while (x < -1) {
                mix_in(1/x);
                if (isinf(p) || p == 0)
                    return p;
                x += 1;
            }
            x += 1;
        }
        x += 1;
    } else {
        while (x >= 2) {
            x -= 1;
            ff = two_mul(p, x);
            p = ff.hi;
            eps += ff.lo / p;
        }
    }
    while (x < 1) {
        ff = two_mul(p, 1/x);
        printf("dividing %La / %La -> %La\n", (long double) p, (long double) x, (long double) ff.hi);
        p = ff.hi;
        if (isinf(p) || p == 0)
            return p;
        eps += ff.lo / p;
        x += 1;
    }

    int         i = sizeof(gamma_coeffs) / sizeof (gamma_coeffs[0]) - 1;
    float_t     r = gamma_coeffs[i];
    float_t     y = x - 1;

    while (--i >= 0)
        r = fma_f(y, r, gamma_coeffs[i]);

    r = r * p * (1 + eps) * 2;
    return r;
}
#endif
