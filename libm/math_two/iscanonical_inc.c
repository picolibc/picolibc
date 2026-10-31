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

#if defined(WANT_FLOAT80) && __LDBL_MANT_DIG__ == 64

#define SNAN_BIT 0x40000000U

int
__iscanonicall(long double value)
{
    int ret;

    FENV_SAVE;
    ieee_extended_shape_type value_bits, canon_bits;

    value_bits.value = value;
    canon_bits.value = value + float_f(-0.0);
    if (isnan(canon_bits.value))
        canon_bits.parts.msw
            = (canon_bits.parts.msw & ~SNAN_BIT) | (value_bits.parts.msw & SNAN_BIT);

    ret = (value_bits.parts.exp == canon_bits.parts.exp)
        && (value_bits.parts.sw == canon_bits.parts.sw);
    FENV_RESTORE;
    return ret;
}

#elif defined(WANT_FLOAT128) && __LDBL_MANT_DIG__ == 106 && __LDBL_MAX_EXP__ == 1024

int
__iscanonicall(long double value)
{
    int ret;

    FENV_SAVE;

    union IEEEl2bits u;
    u.e = value;
    if (!isfinite(value)) {
        ret = (u.dbits.dl == 0.0);
    } else {
        union IEEEl2bits o;

        o.e = value + -0.0l;
        ret = ((u.xbits.expsign == o.xbits.expsign) && (u.xbits.manh == o.xbits.manh)
               && (u.xbits.expsignl == o.xbits.expsignl) && (u.xbits.manl == o.xbits.manl));
    }
    FENV_RESTORE;
    return ret;
}

#endif
