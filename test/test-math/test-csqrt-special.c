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

#include <complex.h>
#include <math.h>
#include <stdio.h>

/*
 * csqrt's special cases (C Annex G.6.4.2): the sign of a zero imaginary
 * part picks the side of the branch cut along the negative real axis,
 * and infinite parts win over NaN ones
 */

static int errors;

#define CHECK(type, func, re, im, want_re, want_im)                                            \
    do {                                                                                       \
        type _Complex z = __builtin_complex((type)(re), (type)(im));                           \
        type _Complex r = func(z);                                                             \
        type r_re = __real__ r, r_im = __imag__ r;                                             \
        type w_re = (type)(want_re), w_im = (type)(want_im);                                   \
        if (!(isnan(w_re) ? isnan(r_re) : (r_re == w_re && !signbit(r_re) == !signbit(w_re)))  \
            || !(isnan(w_im) ? isnan(r_im)                                                     \
                             : (r_im == w_im && !signbit(r_im) == !signbit(w_im)))) {          \
            printf("%s(%g %+gi) = %g %+gi, want %g %+gi\n", #func, (double)(re), (double)(im), \
                   (double)r_re, (double)r_im, (double)w_re, (double)w_im);                    \
            errors++;                                                                          \
        }                                                                                      \
    } while (0)

/*
 * With -ffinite-math-only, the compiler assumes there are no
 * infinities or NaNs, so checks using them aren't meaningful
 */
#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__
#define NONFINITE_CHECKS(type, func) \
    do {                             \
    } while (0)
#else
#define NONFINITE_CHECKS(type, func)                          \
    do {                                                      \
        CHECK(type, func, 1, INFINITY, INFINITY, INFINITY);   \
        CHECK(type, func, 1, -INFINITY, INFINITY, -INFINITY); \
        CHECK(type, func, NAN, INFINITY, INFINITY, INFINITY); \
        CHECK(type, func, -INFINITY, 1, 0, INFINITY);         \
        CHECK(type, func, -INFINITY, -1, 0, -INFINITY);       \
        CHECK(type, func, INFINITY, 1, INFINITY, 0);          \
        CHECK(type, func, INFINITY, -1, INFINITY, -0.0);      \
        CHECK(type, func, INFINITY, NAN, INFINITY, NAN);      \
        CHECK(type, func, 1, NAN, NAN, NAN);                  \
        CHECK(type, func, NAN, 1, NAN, NAN);                  \
    } while (0)
#endif

#define CHECKS(type, func)                     \
    do {                                       \
        CHECK(type, func, -4, 0.0, 0, 2);      \
        CHECK(type, func, -4, -0.0, 0, -2);    \
        CHECK(type, func, 4, -0.0, 2, -0.0);   \
        CHECK(type, func, 0.0, -0.0, 0, -0.0); \
        NONFINITE_CHECKS(type, func);          \
    } while (0)

int
main(void)
{
    CHECKS(double, csqrt);
    CHECKS(float, csqrtf);
/* m68k long double is emulated, and qemu mishandles it; other tests skip it too */
#if defined(_TEST_LONG_DOUBLE) && !defined(__m68k__)
    CHECKS(long double, csqrtl);
#endif
    if (errors) {
        printf("%d errors\n", errors);
        return 1;
    }
    printf("success\n");
    return 0;
}
