/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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

#if __SIZEOF_DOUBLE__ == 8

#define _NEED_IO_FLOAT64

#include "dtoa.h"

int
fcvt_r (double invalue,
        int ndecimal,
        int *decpt,
        int *sign,
        char *buf,
        size_t len)
{
    struct dtoa dtoa;
    int ntrailing;      /* number of zeros to add after the value */
    int ndigit;         /* numer of generated digits */
    int dtoa_decimal = ndecimal;
    char *digits = dtoa.digits;

    if (!isfinite(invalue)) {
        ndigit = 3;
        ntrailing = 0;
        *sign = invalue < 0;
        *decpt = 0;
        if (isnan(invalue))
            digits = "nan";
        else
            digits = "inf";
    } else {
        /* ndecimal = digits after decimal point desired
         * ndigit = digits actually generated
         * dtoa.exp = exponent (position of decimal relative to first digit generated)
         */
        if (ndecimal < 0)
            dtoa_decimal = 0;
        ndigit = __dtoa_engine(invalue, &dtoa, DTOA_MAX_DIG, true, ndecimal);
        *sign = !!(dtoa.flags & DTOA_MINUS);

        /*
         * To compute the number of zeros added after the value, there are
         * three cases:
         *
         * 1. all of the generated digits are left of the decimal
         *    point (dtoa.exp >= ndigit). We need (dtoa.exp - ndigit)
         *    digits left of the decimal and ndecimal right of the
         *    decimal: (dtoa.exp - ndigit) + ndecimal
         *
         * 2. some of the generated digits are right
         *    of the decimal point (dtoa.exp < ndigit). We need
         *    ndecimal digits total, but we have (ndigit - dtoa.exp)
         *    already, so: ndecimal - (ndigit - dtoa.exp).
         *
         * 3. all of the generated digits are right of the decimal point
         *    We need fewer than ndecimal digits by the magnitude of
         *    the exponent (which is negative in this case, so:
         *    ndecimal - (-dtoa.exp - 1) - ndigit
         *
         * These all turn out to be the same computation. Kinda cool.
         */
        ntrailing = (dtoa.exp + 1 - ndigit) + dtoa_decimal;
        /*
         * If this value is negative, then we actually have *no* digits to
         * generate. In this case, we return the empty string and set the
         * exponent to the number of digits requested (as they're all
         * zero)
         */
        if (ntrailing < 0) {
            ntrailing = 0;
            ndigit = 0;

            /*
             * Adjust exponent to reflect the desired output of ndecimal
             * zeros
             */
            dtoa.exp = -(dtoa_decimal + 1);
        }
        *decpt = dtoa.exp + 1;
    }

    /* If we can't fit the whole value in the provided space,
     * return an error
     */
    if ((size_t) (ndigit + ntrailing) >= len)
        return -1;

    /* Value */
    memcpy(buf, digits, ndigit);
    buf += ndigit;

    /* Trailing zeros */
    memset(buf, '0', ntrailing);
    buf += ntrailing;

    /* Null terminate */
    buf[0] = '\0';
    return 0;
}

#elif __SIZEOF_DOUBLE__ == 4

#include "stdio_private.h"

int
fcvt_r (double invalue,
        int ndecimal,
        int *decpt,
        int *sign,
        char *buf,
        size_t len)
{
    return fcvtf_r((float) invalue, ndecimal, decpt, sign, buf, len);
}

#endif
