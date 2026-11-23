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

#define _POSIX_C_SOURCE 202405L
#define _XOPEN_SOURCE   800
#include <math.h>
#include <float.h>
#include <stdio.h>

/*
 * <math.h> for _XOPEN_SOURCE 800 (POSIX.1-2024): every M_ constant,
 * with a long double form ending in 'l', no MAXFLOAT, which Issue 8
 * removed, and no float Bessel functions
 */

#ifdef MAXFLOAT
#error "MAXFLOAT is defined for _XOPEN_SOURCE 800"
#endif

/*
 * XSI specifies the Bessel functions for double alone, so the float
 * ones are the application's names
 */
static int j0f __attribute__((unused));
static int j1f __attribute__((unused));
static int jnf __attribute__((unused));
static int y0f __attribute__((unused));
static int y1f __attribute__((unused));
static int ynf __attribute__((unused));

static int errors;

#define CHECK(name, want)                                           \
    do {                                                            \
        double      _d = (name);                                    \
        long double _l = (name##l);                                 \
        double      _w = (want);                                    \
        long double _wl = (want##L);                                \
        if (fabs(_d - _w) > 2 * DBL_EPSILON * fabs(_w)) {           \
            printf("%s is %.17g, want %.17g\n", #name, _d, _w);     \
            errors++;                                               \
        }                                                           \
        if (fabsl(_l - _wl) > 4 * LDBL_EPSILON * fabsl(_wl)) {      \
            printf("%sl is %.21Lg, want %.21Lg\n", #name, _l, _wl); \
            errors++;                                               \
        }                                                           \
    } while (0)

int
main(void)
{
    CHECK(M_E, 2.718281828459045235360287471352662498);
    CHECK(M_EGAMMA, 0.577215664901532860606512090082402431);
    CHECK(M_LOG2E, 1.442695040888963407359924681001892137);
    CHECK(M_LOG10E, 0.434294481903251827651128918916605082);
    CHECK(M_LN2, 0.693147180559945309417232121458176568);
    CHECK(M_LN10, 2.302585092994045684017991454684364208);
    CHECK(M_PHI, 1.618033988749894848204586834365638118);
    CHECK(M_PI, 3.141592653589793238462643383279502884);
    CHECK(M_PI_2, 1.570796326794896619231321691639751442);
    CHECK(M_PI_4, 0.785398163397448309615660845819875721);
    CHECK(M_1_PI, 0.318309886183790671537767526745028724);
    CHECK(M_1_SQRTPI, 0.564189583547756286948079451560772586);
    CHECK(M_2_PI, 0.636619772367581343075535053490057448);
    CHECK(M_2_SQRTPI, 1.128379167095512573896158903121545172);
    CHECK(M_SQRT2, 1.414213562373095048801688724209698079);
    CHECK(M_SQRT3, 1.732050807568877293527446341505872367);
    CHECK(M_SQRT1_2, 0.707106781186547524400844362104849039);
    CHECK(M_SQRT1_3, 0.577350269189625764509148780501957456);

    /* The double ones are the values correctly rounded */
    {
        /* Stored, so excess precision (m68k, i387) doesn't hide the rounding */
        volatile double sqrt3 = M_SQRT3, sqrt2 = M_SQRT2;
        volatile double want3 = sqrt(3.0), want2 = sqrt(2.0);

        if (sqrt3 != want3 || sqrt2 != want2) {
            printf("M_SQRT3 or M_SQRT2 isn't correctly rounded\n");
            errors++;
        }
    }

    if (errors) {
        printf("%d errors\n", errors);
        return 1;
    }
    printf("success\n");
    return 0;
}
