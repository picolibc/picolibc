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

#if __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__

#define _NEED_IO_LONG_DOUBLE

#include "dtoa.h"

int
ecvtl_r (long double invalue,
         int ndigit,
         int *decpt,
         int *sign,
         char *buf,
         size_t len)
{
    struct dtoa dtoa;
    char *digits = dtoa.digits;
    int ngot;

    if (ndigit < 0)
        ndigit = 0;

    if ((size_t) ndigit > len - 1)
        return -1;

    if (!isfinite(invalue)) {
        *sign = invalue < 0;
        *decpt = 0;
        if (isnan(invalue))
            digits = "nan";
        else
            digits = "inf";
        ngot = 3;
        if (ndigit < ngot)
            ngot = ndigit;
        ndigit = ngot;
    } else {
#ifdef _NEED_IO_FLOAT_LARGE
        ngot = __ldtoa_engine(invalue, &dtoa, ndigit, false, 0);
#elif __SIZEOF_LONG_DOUBLE__ == 8
        ngot = __dtoa_engine((FLOAT64) invalue, &dtoa, ndigit, false, 0);
#elif __SIZEOF_LONG_DOUBLE__ == 4
        ngot = __ftoa_engine((float) invalue, &dtoa, ndigit, false, 0);
#endif
        *sign = !!(dtoa.flags & DTOA_MINUS);
        *decpt = dtoa.exp + 1;
    }
    memset(buf, '0', ndigit);
    memcpy(buf, digits, ngot);
    buf[ndigit] = '\0';
    return 0;
}

#elif __SIZEOF_LONG_DOUBLE__ == 4

#include "stdio_private.h"

int
ecvtl_r (long double invalue,
         int ndigit,
         int *decpt,
         int *sign,
         char *buf,
         size_t len)
{
    return ecvtf_r((float) invalue, ndigit, decpt, sign, buf, len);
}

#elif __SIZEOF_LONG_DOUBLE__ == __SIZEOF_DOUBLE__

#include "stdio_private.h"

int
ecvtl_r (long double invalue,
         int ndigit,
         int *decpt,
         int *sign,
         char *buf,
         size_t len)
{
    return ecvt_r((double) invalue, ndigit, decpt, sign, buf, len);
}

#endif
