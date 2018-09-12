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

#define count_of(n)	(sizeof (n) / sizeof (n[0]))

static const double scale_up[] = {
	1e1,
	1e2,
	1e4,
	1e8,
	1e16,
	1e32,
	1e64,
#if DBL_MAX_10_EXP >= 128
	1e128,
#if DBL_MAX_10_EXP >= 256
	1e256,
#if DBL_MAX_10_EXP >= 512
	1e512,
#if DBL_MAX_10_EXP >= 1024
	1e1024,
#if DBL_MAX_10_EXP >= 2048
	1e2048,
#if DBL_MAX_10_EXP >= 4096
	1e4096,
#if DBL_MAX_10_EXP >= 8192
	1e8192
#endif
#endif
#endif
#endif
#endif
#endif
#endif
};

static const double scale_down[] = {
	1e-1,
	1e-2,
	1e-4,
	1e-8,
	1e-16,
	1e-32,
	1e-64,
#if DBL_MAX_10_EXP >= 128
	1e-128,
#if DBL_MAX_10_EXP >= 256
	1e-256,
#if DBL_MAX_10_EXP >= 512
	1e-512,
#if DBL_MAX_10_EXP >= 1024
	1e-1024,
#if DBL_MAX_10_EXP >= 2048
	1e-2048,
#if DBL_MAX_10_EXP >= 4096
	1e-4096,
#if DBL_MAX_10_EXP >= 8192
	1e-8192
#endif
#endif
#endif
#endif
#endif
#endif
#endif
};

#define paste(a,b) a##e##b
#define expand(a,b) paste(a,b)
#define MIN_MANT (10.0 * expand(1,DBL_DIG))
#define MAX_MANT (10.0 * MIN_MANT)
#define MIN_MANT_INT ((uint64_t) MIN_MANT)

#define max(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a > _b ? _a : _b; })

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

int
__dtoa_engine(double x, struct dtoa *dtoa, int max_digits, int max_decimals)
{
	int	i;
	int32_t	exp = 0;
	uint8_t	flags = 0;
	double	y;

	if (x < 0) {
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
		exp = DBL_DIG + 1;

		/* Bring x within range MIN_MANT <= x < MAX_MANT while
		 * computing exponent value
		 */
		if (x < MIN_MANT) {
			for (i = count_of(scale_up) - 1; i >= 0; i--) {
				y = x * scale_up[i];
				if (y < MAX_MANT) {
					x = y;
					exp -= (1 << i);
				}
			}
		} else {
			for (i = count_of(scale_down) - 1; i >= 0; i--) {
				y = x * scale_down[i];
				if (y >= MIN_MANT) {
					x = y;
					exp += (1 << i);
				}
			}
		}

		/* Now convert mantissa to decimal.
		 */

		uint8_t		outpos = 0;
		uint64_t	mant = (uint64_t) (x + 0.5);
		uint64_t	decimal = MIN_MANT_INT;

		/* If limiting decimals, then limit the max digits
		 * to no more than the number of digits left of the decimal
		 * plus the number of digits right of the decimal
		 */

		if(max_decimals != 0)
			max_digits = min(max_digits, max_decimals + max(exp + 1, 1));

		/* Compute digits */
		for (outpos = 0; outpos < max_digits; outpos++) {
			dtoa->digits[outpos] = mant / decimal + '0';
			mant %= decimal;
			decimal /= 10;
		}

		/* Round */
		if (mant >= decimal * 5) {
			while(outpos > 0) {

				/* Increment digit, check if we're done */
				if(++dtoa->digits[outpos-1] < '0' + 10)
					break;

				/* Rounded past the first digit;
				 * reset the leading digit to 1,
				 * bump exp and tell the caller we've carried.
				 * The remaining digits will already be '0',
				 * so we don't need to mess with them
				 */
				if(outpos == 0)
				{
					dtoa->digits[0] = '1';
					exp++;
					flags |= DTOA_CARRY;
					break;
				}

				dtoa->digits[outpos-1] = '0';
				outpos--;

				/* and the loop continues, carrying to next digit. */
			}
		}

	}
	dtoa->digits[max_digits] = '\0';
	dtoa->flags = flags;
	dtoa->exp = exp;
	return max_digits;
}
