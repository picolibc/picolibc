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

#include <math.h>
#include <stdio.h>
#include <inttypes.h>

typedef struct {
    float       hi;
    float       lo;
} ff_t;

extern float _ff_scalbnf(ff_t x, int n);

static float
float_from_uint32(uint32_t u32)
{
    union {
        float           f32;
        uint32_t        u32;
    } u;

    u.u32 = u32;
    return u.f32;
}

static uint32_t
uint32_from_float(float f32)
{
    union {
        float           f32;
        uint32_t        u32;
    } u;

    u.f32 = f32;
    return u.u32;
}

#define START   0x3f800000UL
#define END     0x40000000UL
#define STEP    0x00000001UL

int main(void)
{
    uint32_t    u32 = START;
    int         ret = 0;

    for (;;) {
        float   fx = float_from_uint32(u32);
        float   lx = fx * 0x1p-40f;
        float   x_sign, y_sign;
        int     n;

        for (x_sign = -1; x_sign <= 1; x_sign += 2) {
            for (y_sign = 1; y_sign <= 1; y_sign += 2) {
                ff_t    ffx = { .hi = fx * x_sign, .lo = lx * y_sign };
                double  dx = (double) fx * (double) x_sign + (double) lx * (double) y_sign;

                for (n = -152; n <= -126; n++) {
                    float      dy = (float) scalbn(dx, n);
                    float      fy = _ff_scalbnf(ffx, n);
                    if (dy != fy) {
                        printf("0x%08x 0x%08x %d -> 0x%08x 0x%08x\n",
                               uint32_from_float(ffx.hi), uint32_from_float(ffx.lo), n,
                               uint32_from_float(fy), uint32_from_float(dy));
                        ret = 1;
                    }
                }
            }
        }
        if ((u32 & 0xffffff) == 0)
            printf("    0x%08" PRIx32 "\n", u32);

        if (u32 == END)
            break;
        u32 += STEP;
    }
    return ret;
}
