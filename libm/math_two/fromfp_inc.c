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
#include <fenv.h>

#ifdef float_t

#define FLOATFP_MAX_EXP (FLOAT_MAX_EXP + 1)

/*
 * Targets with built-in rounding functions will sometimes
 * raise FE_INEXACT exceptions. List those targets here and
 * save/restore that bit as needed
 */

/* x86 with SSE */
#if defined(__SSE__)
#define PRESERVE_INEXACT
#endif

/* risc-v with an FPU of sufficient width */
#if defined(__riscv_flen) && __riscv_flen >= FLOAT_LEN
#define PRESERVE_INEXACT
#endif

#ifdef PRESERVE_INEXACT
#define SAVE int save_except = fetestexcept(FE_INEXACT)
#define RESTORE                        \
    do {                               \
        if (!save_except)              \
            feclearexcept(FE_INEXACT); \
    } while (0)
#else
#define SAVE
#define RESTORE
#endif

float_t
name(NAME)(float_t x, int rnd, unsigned int width)
{
    float_t xi, sig;
    int     e;

    if (!isfinite(x) || width == 0)
        return kernel(__math_invalid)(x);

    SAVE;
    switch (rnd) {
    case FP_INT_UPWARD:
        xi = name(ceil)(x);
        break;
    case FP_INT_DOWNWARD:
        xi = name(floor)(x);
        break;
    default:
        /*
         * Unknown rounding directions -- follow glibc and use
         * truncation
         */
    case FP_INT_TOWARDZERO:
        xi = name(trunc)(x);
        break;
    case FP_INT_TONEARESTFROMZERO:
        xi = name(round)(x);
        break;
    case FP_INT_TONEAREST:
        xi = name(roundeven)(x);
        break;
    }
    RESTORE;

    if (width > FLOATFP_MAX_EXP)
        width = FLOATFP_MAX_EXP;
#ifdef UNSIGNED
    if (xi < 0)
        return kernel(__math_invalid)(x);
    width++;
#endif

    sig = name(frexp)(xi, &e);

    unsigned int eu = (unsigned int)e;

    /* check for overflow; special case for negative power of two */
    if (eu > width || (eu == width && sig != float_f(-0.5)))
        return kernel(__math_invalid)(x);

#ifdef INEXACT
    if (xi != x)
        return kernel(__math_inexact)(xi);
#endif

    return xi;
}

#if defined(WANT_FLOAT32)
_MATH_ALIAS_f_fiu(NAME)
#elif defined(WANT_FLOAT64)
_MATH_ALIAS_d_diu(NAME)
#endif

#endif
