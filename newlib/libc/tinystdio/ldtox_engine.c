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

/*
 * Convert an 80-bit or 128-bit float to a string in hex ('a') format
 * 
 * This chunk of code gets inserted into the vfprintf function in the
 * long double case when long double is larger than 64 bits.
 *
 * This code uses the 'u128' type to hold the floating point value as
 * an integer value.
 *
 * This code only works with long double type.
 */

#if __SIZEOF_LONG_DOUBLE__ > 8

#define _NEED_IO_FLOAT_LARGE

#include "dtoa.h"

#if __LDBL_MANT_DIG__ == 64
# define LEXP_BIAS      (__LDBL_MAX_EXP__ + 2)
# define LEXP_INF       (__LDBL_MAX_EXP__ - 3)
# define LSIG_BITS      (__LDBL_MANT_DIG__)
# ifdef __m68k__
#  define LDENORM_EXP_BIAS 0
# else
#  define LDENORM_EXP_BIAS 1
#  define LSIG_MSB_INF    _u128_lshift(to_u128(1), __LDBL_MANT_DIG__-1)
# endif
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define LEXP_SHIFT       __LDBL_MANT_DIG__
#  define LSIGN_SHIFT        79
# endif
# if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define LEXP_SHIFT       (__LDBL_MANT_DIG__ + 16)
#  define LSIGN_SHIFT        (79 + 16)
# endif
#else
# define LDENORM_EXP_BIAS 1
# define LSIGN_SHIFT        127
# define LEXP_BIAS       (__LDBL_MAX_EXP__ - 1)
# define LEXP_INF        (__LDBL_MAX_EXP__)
# define LSIG_MSB        _u128_lshift(to_u128(1), __LDBL_MANT_DIG__-1)
# define LSIG_BITS       (__LDBL_MANT_DIG__ - 1)
# define LEXP_SHIFT      (__LDBL_MANT_DIG__ - 1)
#endif

#define LEXP_MASK        ((__LDBL_MAX_EXP__ - 1) + __LDBL_MAX_EXP__)
#define LSIG_SHIFT       0
#define LSIG_MASK        _u128_minus_64(_u128_lshift(to_u128(1), LSIG_BITS), 1)

#define TOCASE(c)       ((c) - case_convert)

#define LDTOX_NDIGS     (__LDBL_MANT_DIG__ + 3) / 4

int
__ldtox_engine(long double x, struct dtoa *dtoa, int prec, unsigned char case_convert)
{
    _u128 fi, s;
    int exp;

    fi = asuintld(x);

    exp = _u128_and_64(_u128_rshift(fi, LEXP_SHIFT), LEXP_MASK);
    s = fi = _u128_lshift(_u128_and(fi, LSIG_MASK), LSIG_SHIFT);
    if (!_u128_is_zero(s) || exp != 0) {
        if (exp == 0)
            exp = LDENORM_EXP_BIAS;
        else
        {
#ifdef LSIG_MSB
            s = _u128_or(s, LSIG_MSB);
#endif
        }
        exp -= LEXP_BIAS;
    }
    dtoa->flags = 0;
    if (_u128_and_64(_u128_rshift(fi, LSIGN_SHIFT), 1))
        dtoa->flags = DTOA_MINUS;

    if (prec < 0)
        prec = 0;
    else if (prec >= (LDTOX_NDIGS - 1))
        prec = LDTOX_NDIGS - 1;
    else {
        int     bits = ((LDTOX_NDIGS - 1) - prec) << 2;
        _u128   half = _u128_lshift(to_u128(1), bits - 1);
        _u128   mask = _u128_not(_u128_minus_64(_u128_lshift(half, 1), 1));

        /* round even */
        if (_u128_gt(_u128_and(s, _u128_not(mask)), half) || _u128_and_64(_u128_rshift(s, bits), 1) != 0)
            s = _u128_plus(s, half);

        s = _u128_and(s, mask);
    }

    if (exp == LEXP_INF) {
#ifdef LSIG_MSB_INF
        if (!_u128_eq(fi, LSIG_MSB_INF))
#else
            if (!_u128_is_zero(fi))
#endif
                dtoa->flags |= DTOA_NAN;
            else
                dtoa->flags |= DTOA_INF;
    } else {
        int8_t d;
        for (d = LDTOX_NDIGS - 1; d >= 0; d--) {
            int dig = _u128_and_64(s, 0xf);
            s = _u128_rshift(s, 4);
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
