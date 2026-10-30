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

/*
 * Code shared between fma and the narrowing ffma and dfma functions.
 * Include after math_two.h with float_t defined.
 */

/*
 * Compute x * y + z for finite x, y and z where x and y are non-zero.
 * The operands are scaled with frexp so nothing overflows or
 * underflows; the result is (hi + lo) * 2**e, normalized so that hi
 * is the nearest value to the sum and lo has been rounded to odd, so
 * that hi + lo can be rounded once more without double rounding
 * error. When hi is zero, the high parts cancelled exactly and the
 * result is exactly lo. The rounding mode must be round to nearest.
 */
static math_two_inline ff_t
name(_fma_scaled)(float_t x, float_t y, float_t z, int *e)
{
    float_t xs, ys, zs;
    int     xe, ye, ze;

    xs = name(frexp)(x, &xe);
    ys = name(frexp)(y, &ye);
    *e = xe + ye;

    /*
     * fma handles z == 0 itself; this is for ffma and dfma, which
     * need the exact product to avoid rounding twice
     */
    if (z == 0)
        return f_mul_f(xs, ys);

    zs = name(frexp)(z, &ze);

    int spread = xe + ye - ze;

    /* When x * y is far below z, it only acts as a sticky bit */
    if (spread < -FLOAT_MANT_DIG) {
        *e = ze;
        return (ff_t) { .hi = zs, .lo = name(copysign)(FLOAT_MIN, xs * ys) };
    }

    /* Likewise when z is far below x * y */
    if (spread <= FLOAT_MANT_DIG * 2)
        zs = _ldexp_no_errno(zs, -spread);
    else
        zs = name(copysign)(FLOAT_MIN, zs);

    ff_t xy = f_mul_f(xs, ys);
    ff_t rr = f_add_f(xy.hi, zs);

    if (rr.hi == 0)
        return (ff_t) { .hi = 0, .lo = xy.lo };

    /* Renormalize so that a non-zero lo means the result is inexact */
    return f_add_f(rr.hi, round_odd_ff(f_add_f(xy.lo, rr.lo)));
}
