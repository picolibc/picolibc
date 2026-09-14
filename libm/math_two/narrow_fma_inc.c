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
#define func       name(dfma)
#define INVALID(x) __math_invalid(x)
#else
typedef float narrow_t;
#define func       name(ffma)
#define INVALID(x) __math_invalidf(x)
#endif

narrow_t
func(float_t x, float_t y, float_t z)
{
    /*
     * Handle special cases. The order of operations and the particular
     * return values here are crucial in handling special cases involving
     * infinities, NaNs, overflows, and signed zeroes correctly.
     */
    if (!isfinite(z) && isfinite(x) && isfinite(y))
        return (narrow_t)(z + z);
    if (!isfinite(x) || !isfinite(y) || !isfinite(z))
        return (narrow_t)(x * y + z);
    if (x == (float_t)0.0 || y == (float_t)0.0)
        return (narrow_t)(x * y + z);

    ff_t rr = f_mul_f(x, y);
    if (z != (float_t)0.0)
        rr = ff_add_f(rr, z);

    return (narrow_t)round_odd_ff(rr);
}

#endif
