/* Copyright © 2018, Keith Packard
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#include "dtoa_engine.h"
#include <math.h>

typedef double dtoa_type;
#define DTOA_DIG DBL_DIG
#define DTOA_MAX_10_EXP DBL_MAX_10_EXP
#define DTOA_MIN_10_EXP DBL_MIN_10_EXP

#include "dtoa_data.c"

/* A bit of CPP trickery -- construct the floating-point value 10 ** DTOA_DIG
 * by pasting the value of DTOA_DIG onto '1e' to
 */

#define paste(a) 1e##a
#define substitute(a) paste(a)
#define MIN_MANT (substitute(DTOA_DIG))
#define MAX_MANT (10.0 * MIN_MANT)
#define MIN_MANT_INT ((uint64_t) MIN_MANT)
#define MIN_MANT_EXP	DTOA_DIG

#define count_of(n)	(sizeof (n) / sizeof (n[0]))

#define max(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a > _b ? _a : _b; })

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

int
__dtoa_engine(dtoa_type x, struct dtoa *dtoa, int max_digits, int max_decimals)
{
	int	i;
	uint8_t	flags = 0;
	int32_t	exp = 0;

	if (signbit(x)) {
		flags |= DTOA_MINUS;
		x = -x;
	}
	if (x == 0) {
		flags |= DTOA_ZERO;
		for (i = 0; i < max_digits; i++)
			dtoa->digits[i] = '0';
	} else if (isnan(x)) {
		flags |= DTOA_NAN;
	} else if (isinf(x)) {
		flags |= DTOA_INF;
	} else {
		dtoa_type	y;

		exp = MIN_MANT_EXP;

		/* Bring x within range MIN_MANT <= x < MAX_MANT while
		 * computing exponent value
		 */
		if (x < MIN_MANT) {
			for (i = count_of(dtoa_scale_up) - 1; i >= 0; i--) {
				y = x * dtoa_scale_up[i];
				if (y < MAX_MANT) {
					x = y;
					exp -= (1 << i);
				}
			}
		} else {
			for (i = count_of(dtoa_scale_down) - 1; i >= 0; i--) {
				y = x * dtoa_scale_down[i];
				if (y >= MIN_MANT) {
					x = y;
					exp += (1 << i);
				}
			}
		}

		/* If limiting decimals, then limit the max digits
		 * to no more than the number of digits left of the decimal
		 * plus the number of digits right of the decimal
		 */

		if(max_decimals != 0)
			max_digits = min(max_digits, max_decimals + max(exp + 1, 1));

		/* Round nearest by adding 1/2 of the last digit
		 * before converting to int. Check for overflow
		 * and adjust mantissa and exponent values
		 */

		x = x + dtoa_round[max_digits];

		if (x >= MAX_MANT) {
			x /= 10.0;
			exp++;
		}

		/* Now convert mantissa to decimal. */

		uint64_t	mant = (uint64_t) x;
		uint64_t	decimal = MIN_MANT_INT;

		/* Compute digits */
		for (i = 0; i < max_digits; i++) {
			dtoa->digits[i] = mant / decimal + '0';
			mant %= decimal;
			decimal /= 10;
		}
	}
	dtoa->digits[max_digits] = '\0';
	dtoa->flags = flags;
	dtoa->exp = exp;
	return max_digits;
}
