/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#define _NEED_IO_LONG_DOUBLE

#include "dtoa.h"

#ifdef _NEED_IO_FLOAT_LARGE

#define max(a, b) ({\
		__typeof(a) _a = a;\
		__typeof(b) _b = b;\
		_a > _b ? _a : _b; })

#define min(a, b) ({\
		__typeof(a) _a = a;\
		__typeof(b) _b = b;\
		_a < _b ? _a : _b; })

int
__ldtoa_engine(long double x, struct dtoa *dtoa, int max_digits, bool fmode, int max_decimals)
{
    int i;

    dtoa->flags = 0;

    if (signbit(x)) {
        dtoa->flags |= DTOA_MINUS;
        x = -x;
    }
    if (isnan(x)) {
        dtoa->flags |= DTOA_NAN;
        return max_digits;
    } else if (isinf(x)) {
        dtoa->flags |= DTOA_INF;
        return max_digits;
    }

    int expo;
    int decexp = 0;

    x = frexpl(x, &expo);

    /* move into [1-2) range */
    x *= 2.0l;
    expo--;

    while (expo <= -2) {

        /* Make room to allow multiplication by 5 without overflow */
        do {
            expo++;
            x *= 0.5l;
        } while (x >= 2.0l);

        x *= 5.0l;
        expo++;
        decexp--;
    }

    while (expo > 0) {
        /* divide by 10 */
        x *= 0.2l;
        expo--;
        decexp++;

        /* get the value back in range */
        do {
            x *= 2.0l;
            expo--;
        } while (x < 1.0l);
    }

    while (expo < 0) {
        x *= 0.5l;
        expo++;
    }

    if (x < 1.0l) {
        x *= 10.0l;
        decexp--;
    }

    /* x is now in the range 1 <= x < 10 */

    int save_max_digits = max_digits;

    if (fmode)
        max_digits = min(max_digits, max(1, max_decimals + decexp + 1));

    int decimals = max_digits;

    long double round = 0.5l;
    while (decimals--) {
        round *= 0.1l;
    }
    x += round;

    if (x >= 10.0l) {
        x *= 0.1l;
        decexp++;
        max_digits = min(save_max_digits, max(1, max_decimals + decexp + 1));
    }

    /* convert to decimal */

    for (i = 0; i < max_digits; i++) {
        int digit = (int) x;
        dtoa->digits[i] = '0' + digit;
        x -= digit;
        x *= 10.0l;
    }

    dtoa->exp = decexp;

    return max_digits;
}

#endif
