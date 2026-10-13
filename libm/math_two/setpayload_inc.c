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
#include <string.h>

#ifdef float_t

#ifndef SIG
#define SIG 0
#endif

#if SIG
#define NAME setpayloadsig
#else
#define NAME setpayload
#endif

#define SET_SNAN (!SIG != !_IEEE_754_2008_SNAN)

#ifdef WANT_FLOAT80
#define EXPLICIT_BIT 1
#else
#define EXPLICIT_BIT 0
#endif

int
name(NAME)(float_t *result, float_t payload)
{
    fuint_t pi = f_touint(payload);
    int     exponent = f_exp(pi);

    bool    valid = true;

    /* Negative values are invalid */
    if (f_is_minus(pi))
        valid = false;

    /*
     * Values larger than can be held in the significant without using
     * the SNAN/QNAN bit are invalid.
     */
    if (exponent >= (FLOAT_PAYLOAD_DIG - 1 - EXPLICIT_BIT))
        valid = false;

    /*
     * A negative exponent is invalid if the value isn't zero or we're
     * not setting the SNAN/QNAN bit.
     */
    if (valid && exponent < 0) {
        if (!f_is_zero(pi) || !SET_SNAN)
            valid = false;
    }

    int shift = FLOAT_PAYLOAD_DIG - exponent;

    /*
     * A value with any fractional component is invalid
     */
    if (valid && !f_is_integer(pi, shift))
        valid = false;

    if (!valid) {
        *result = 0;
        return 1;
    }

#ifdef fuint_t
    if (!f_is_zero(pi)) {
        /* Shift the payload into place */
        pi &= ((fuint_t)1 << FLOAT_PAYLOAD_DIG) - 1;
        pi |= (fuint_t)1 << FLOAT_PAYLOAD_DIG;
        pi >>= shift;
    }

    /* Add in the exponent value */
    pi |= (fuint_t)FLOAT_EXP_INF << FLOAT_PAYLOAD_DIG;

    /* Set the SNAN/QNAN bit if necessary */
    if (SET_SNAN)
        pi |= FLOAT_SNAN_BIT;
#ifdef __i386__
    memcpy(result, &pi, sizeof(*result));
#else
    *result = f_tofloat(pi);
#endif
#else
#ifdef WANT_FLOAT80
    if (!f_is_zero(pi))
        /* Shift the payload into place */
        pi.parts.sw >>= (shift - 1);

    /* Add the leading one */
    pi.parts.msw |= LDBL_NBIT;

    /* Add in the exponent value */
    pi.parts.exp = FLOAT_EXP_INF;

    /* Set the SNAN/QNAN bit if necessary */
    if (SET_SNAN)
        pi.parts.sw |= FLOAT_SNAN_BIT;
    memcpy(result, &pi, sizeof(*result));
#endif
#ifdef WANT_FLOAT128
    if (!f_is_zero(pi)) {
        pi.parts64.msw &= ((uint64_t)1 << (FLOAT_PAYLOAD_DIG - 64)) - 1;
        pi.parts64.msw |= ((uint64_t)1 << (FLOAT_PAYLOAD_DIG - 64));
        if (shift >= 64) {
            pi.parts64.lsw = pi.parts64.msw >> (shift - 64);
            pi.parts64.msw = 0;
        } else {
            pi.parts64.lsw = (pi.parts64.lsw >> shift) | (pi.parts64.msw << (64 - shift));
            pi.parts64.msw = pi.parts64.msw >> shift;
        }
    }

    /* Add in the exponent value */
    pi.parts64.msw |= (uint64_t)FLOAT_EXP_INF << (FLOAT_PAYLOAD_DIG - 64);

    /* Set the SNAN/QNAN bit if necessary */
    if (SET_SNAN)
        pi.parts64.msw |= FLOAT_SNAN_BIT;
    *result = f_tofloat(pi);
#endif
#endif

    return 0;
}

#endif
