/* $NetBSD: casinhl.c,v 1.1 2014/10/10 00:48:18 christos Exp $ */

/*-
 * Copyright (c) 2007 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software written by Stephen L. Moshier.
 * It is redistributed by the NetBSD Foundation by permission of the author.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <complex.h>
#include <math.h>
#include <float.h>

#ifdef __HAVE_LONG_DOUBLE_MATH

long double complex
casinhl(long double complex z)
{

    long double x = fabsl(creall(z));
    long double y = fabsl(cimagl(z));
    long double complex res;
    long double complex w;

    const long double eps = LDBL_EPSILON;

    if (y == 0.0L) {
        if (isnan(x)) {
            res = CMPLXL(NAN, copysignl(0.0L, cimagl(z)));
        }
        else if (isinf(x)) {
            res = CMPLXL(x, copysignl(0.0L, cimagl(z)));
        }
        else {
            res = CMPLXL(asinhl(x), copysignl(0.0L, cimagl(z)));
        }
    }
    /* Handle large values */
    else if (x >= 1.0L/eps || y >= 1.0L/eps) {
        res = clogl(CMPLXL(x, y));
        res = CMPLXL(creall(res) + _M_LN2_LD, cimagl(res));

    }

    /* Case where real part >= 0.5 and imag part very samll */
    else if (x >= 0.5L && y < eps/8.0L) {
        long double s = hypotl(1.0L, x);
        res = CMPLXL(logl(x + s), atan2l(y, s));
    }

    /* Case Where real part very small and imag part >= 1.5 */
    else if (x < eps/8.0L && y >= 1.5L) {
        long double s = sqrtl((y + 1.0L) * (y - 1.0L));
        res = CMPLXL(logl(y + s), atan2l(s, x));
    }

    else {
        /* General case */
        w = CMPLXL((x - y) * (x + y) + 1.0L, (2.0L * x * y));
        w = csqrtl(w);

        w = CMPLXL(x + creall(w), y + cimagl(w));
        res = clogl(w);
    }

    /* Apply correct signs */
    res = CMPLXL(copysignl(creall(res), creall(z)),
                 copysignl(cimagl(res), cimagl(z)));

    return res;
}

#endif
