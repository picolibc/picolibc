/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#ifndef DTOX_SIZE
#define DTOX_SIZE 8
#endif

#include "stdio_private.h"

#if DTOX_SIZE == 4 || defined(FLOAT64)

#if DTOX_SIZE == 8

#define DTOA_MAX_DIG            17
#define DTOA_MAX_10_EXP         308
#define DTOA_MIN_10_EXP         (-307)
#define DTOA_SCALE_UP_NUM       9
#define DTOA_ROUND_NUM          (DTOA_MAX_DIG + 1)

#define DTOX_UINT       uint64_t
#define DTOX_INT        int64_t
#define DTOX_FLOAT      FLOAT64
#define DTOX_ASUINT(x)  asuint64(x)

#define EXP_SHIFT       52
#define EXP_MASK        0x7ff
#define SIG_SHIFT       0
#define SIG_MASK        0xfffffffffffffLL
#define EXP_BIAS        1023
#define ASUINT(x)       asuint64(x)
#define DTOX_NDIGS 14

#define _NEED_IO_FLOAT64

#elif DTOX_SIZE == 4

#define _NEED_IO_FLOAT32
#define __dtox_engine __ftox_engine

#define DTOX_UINT       uint32_t
#define DTOX_INT        int32_t
#define DTOX_FLOAT      float
#define DTOX_ASUINT(x)  asuint(x)

#define EXP_SHIFT       23
#define EXP_MASK        0xff
#define SIG_SHIFT       1
#define SIG_MASK        0x7fffff
#define EXP_BIAS        127
#define ASUINT(x)       asuint(x)
#define DTOX_NDIGS 7
#endif

#include "dtoa.h"

/*
 * Convert a 32-bit or 64-bit float to a string in hex ('a') format
 *
 * This chunk of code gets inserted into the vfprintf function in the
 * double/float handling code as well as the long double case when
 * long double is 32- or 64- bits.
 *
 * This code assumes that there is an integer type suitable for holding
 * the entire floating point value.
 *
 * Define DTOX_UINT, DTOX_INT, DTOX_FLOAT and DTOX_SIZE before including
 * this file.
 */

#define TOCASE(c)       ((c) - case_convert)

int
__dtox_engine (DTOX_FLOAT x, struct dtoa *dtoa, int prec, unsigned char case_convert)
{
    DTOX_INT   fi, s;
    int exp;

    fi = ASUINT(x);

    dtoa->digits[0] = '0';

    exp = ((fi >> EXP_SHIFT) & EXP_MASK);
    s = (fi & SIG_MASK) << SIG_SHIFT;
    if (s | exp) {
        if (!exp)
            exp = 1;
        else
            dtoa->digits[0] = '1';
        exp -= EXP_BIAS;
    }
    dtoa->flags = 0;
    if (fi < 0)
        dtoa->flags = DTOA_MINUS;

    if (prec < 0)
        prec = 0;
    else if (prec >= (DTOX_NDIGS - 1))
        prec = DTOX_NDIGS - 1;
    else {
        int                 bits = ((DTOX_NDIGS - 1) - prec) << 2;
        DTOX_INT            half = ((DTOX_INT) 1) << (bits - 1);
        DTOX_INT            mask = ~((half << 1) - 1);

        /* round even */
        if ((s & ~mask) > half || ((s >> bits) & 1) != 0)
            s += half;
        /* special case rounding first digit */
        if (s > (SIG_MASK << SIG_SHIFT))
            dtoa->digits[0]++;
        s &= mask;
    }

    if (exp == EXP_BIAS + 1) {
        if (s)
            dtoa->flags |= DTOA_NAN;
        else
            dtoa->flags |= DTOA_INF;
    } else {
        int d;
        for (d = DTOX_NDIGS - 1; d; d--) {
            int dig = s & 0xf;
            s >>= 4;
            if (dig == 0 && d > prec)
                continue;
            if (dig <= 9)
                dig += '0';
            else
                dig += TOCASE('a' - 10);
            dtoa->digits[d] = dig;
            if (prec < d)
                prec = d;
        }
    }
    dtoa->exp = exp;
    return prec;
}

#endif
