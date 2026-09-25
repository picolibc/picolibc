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

#ifdef NARROW_D
typedef double narrow_t;
#define func       name(dsqrt)
#define INVALID(x) __math_invalid(x)
#else
typedef float narrow_t;
#define func       name(fsqrt)
#define INVALID(x) __math_invalidf(x)
#endif

/*
 * A fairly straightforward Newton's method
 * implementation for sqrt.
 */

narrow_t
func(float_t x)
{
    if (x < 0)
        return INVALID(x);
    if (!isfinite(x))
        return x + x;
    if (x == 0)
        return x;

    float_t frac;
    int     exp;

    frac = name(frexp)(x, &exp);

    ff_t yy = f_to_ff(name(ldexp)(frac / 2, exp / 2));
    ff_t xx = f_to_ff(x);

    int  loop;
    int  bounds = 128;

    for (loop = 0; loop < bounds; loop++) {
        ff_t qq = ff_div_ff(xx, yy);
        ff_t ss = ff_add_ff(yy, qq);
        ff_t nn = { .hi = ss.hi / 2, .lo = ss.lo / 2 };

        /*
         * Once the upper term is stable, run the loop twice more to
         * compute the lower term
         */
        if (bounds > loop + 2 && nn.hi == yy.hi)
            bounds = loop + 2;

        yy = nn;
    }

    ff_t test = f_mul_f(yy.hi, yy.hi);
    if (test.hi == x && test.lo == 0)
        yy.lo = 0;

    return (narrow_t)round_odd_ff(yy);
}

#endif
