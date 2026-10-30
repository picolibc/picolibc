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

#include "fma_common_inc.c"

#ifdef NARROW_D
typedef double narrow_t;
#define narrow(x)  narrow_to_double(x)
#define func       name(dfma)
#define INVALID(x) __math_invalid(x)
#else
typedef float narrow_t;
#define narrow(x)  narrow_to_float(x)
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
        return narrow(z + z);
    if (!isfinite(x) || !isfinite(y) || !isfinite(z))
        return narrow(x * y + z);
    if (x == (float_t)0.0 || y == (float_t)0.0)
        return narrow(x * y + z);

    /*
     * Compute x * y + z rounded to odd at the wider precision in
     * round to nearest mode, then scale and narrow the result in the
     * caller's rounding mode. Rounding to odd first ensures the final
     * rounding is correct in every mode
     */
    int  e;
    ff_t rr;

    SAVE_ENV;

    /*
     * The barriers keep the compiler from moving the computation
     * across the rounding mode changes, including the computations
     * after the rounding mode is restored.
     */
    rr = name(_fma_scaled)(name(force_eval)(x), y, z, &e);
    if (rr.hi == 0) {
        /* An exact zero takes its sign from the rounding mode */
        if (rr.lo == 0) {
            RESTORE;
            return narrow(name(force_eval)(x) * y + z);
        }
        rr = (ff_t) { .hi = rr.lo, .lo = 0 };
    }

    float_t r = name(force_eval)(round_odd_ff(rr));

    RESTORE;
    return narrow(_ldexp_no_errno(name(force_eval)(r), e));
}

#endif
