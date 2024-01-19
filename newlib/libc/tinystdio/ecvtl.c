/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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

#if __SIZEOF_LONG_DOUBLE__ >8

#define _NEED_IO_LONG_DOUBLE

#include "dtoa.h"

static NEWLIB_THREAD_LOCAL char ecvt_buf[LDTOA_MAX_DIG + 1];

char *
ecvtl (long double invalue,
       int ndigit,
       int *decpt,
       int *sign)
{
    if (ecvtl_r(invalue, ndigit, decpt, sign, ecvt_buf, sizeof(ecvt_buf)) < 0)
        return NULL;
    return ecvt_buf;
}


#elif __SIZEOF_LONG_DOUBLE__ == 4

#include "stdio_private.h"

char *
ecvtl (long double invalue,
       int ndigit,
       int *decpt,
       int *sign)
{
    return ecvtf((float) invalue, ndigit, decpt, sign);
}

#elif __SIZEOF_LONG_DOUBLE__ == __SIZEOF_DOUBLE__

#include "stdio_private.h"

char *
ecvtl (long double invalue,
       int ndigit,
       int *decpt,
       int *sign)
{
    return ecvt((double) invalue, ndigit, decpt, sign);
}

#endif

