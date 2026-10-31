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
#define WANT_DOUBLE_DOUBLE
#include "math_two.h"

#ifdef float_t

int
name(canonicalize)(float_t *result_p, const float_t *input_p)
{
#if defined(WANT_FLOAT80) || (defined(WANT_FLOAT128) && defined(_DOUBLE_DOUBLE_FLOAT))
    FENV_SAVE;
    float_t input = *input_p;

    if (isnan(input)) {
#ifdef _DOUBLE_DOUBLE_FLOAT
        union IEEEl2bits u;

        u.e = input;
        u.dbits.dl = 0.0;
        input = u.e;
#endif
        FENV_RESTORE;
        *result_p = kernel(__math_invalid)(input);
    } else {
        *result_p = input + float_f(-0.0);
        FENV_RESTORE;
    }
    return 0;
#else
    float_t input = *input_p;

    if (issignaling(input))
        input = input + input;
    *result_p = input;
    return 0;
#endif
}

#endif
