/* $NetBSD: casinhf.c,v 1.1 2007/08/20 16:01:32 drochner Exp $ */

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
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

#include <complex.h>
#include <math.h>
#include <float.h>

float complex
casinhf(float complex z)
{

    float x = fabsf(crealf(z));
    float y = fabsf(cimagf(z));
    float complex res;
    float complex w;

    const float eps = FLT_EPSILON;

    if (y == 0.0f) {
        if (isnanf(x)) {
            res = NAN + copysignf(0.0, cimagf(z)) * I;
        }
        else if (isinff(x)) {
            res = x + copysignf(0.0, cimagf(z)) * I;
        }
        else {
            res = asinhf(x) + copysignf(0.0, cimagf(z)) * I;
        }
    }
    /* Handle large values */
    else if (x >= 1.0f/eps || y >= 1.0f/eps) {
        res = clogf(x + y * I);
        res = crealf(res) + (float) _M_LN2 + cimagf(res) * I;

    }

    /* Case where real part >= 0.5 and imag part very samll */
    else if (x >= 0.5f && y < eps/8.0f) {
        float s = hypotf(1.0f, x);
        res = logf(x + s) + atan2f(y, s) * I;
    }

    /* Case Where real part very small and imag part >= 1.5 */
    else if (x < eps/8.0f && y >= 1.5f) {
        float s = sqrtf((y + 1.0f) * (y - 1.0f));
        res = logf(y + s) + atan2f(s, x) * I;
    }

    else {
        /* General case */
        w = (x - y) * (x + y) + 1.0f + (2.0f * x * y) * I;
        w = csqrtf(w);

        w = (x + crealf(w)) + (y + cimagf(w)) * I;
        res = clogf(w);
    }

    /* Apply correct signs */
    res = copysignf(crealf(res), crealf(z)) +
          copysignf(cimagf(res), cimagf(z)) * I;

    return res;
}
