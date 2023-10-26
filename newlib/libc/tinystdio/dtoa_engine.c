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

#include "stdio_private.h"

#ifdef FLOAT64

#define _NEED_IO_FLOAT64

#include "dtoa.h"

#define max(a, b) ({\
		__typeof(a) _a = a;\
		__typeof(b) _b = b;\
		_a > _b ? _a : _b; })

#define min(a, b) ({\
		__typeof(a) _a = a;\
		__typeof(b) _b = b;\
		_a < _b ? _a : _b; })

#define FRACTION_BITS   (53 - 1)
#define FRACTION_MASK   (((uint64_t) 1 << FRACTION_BITS) - 1)
#define EXPONENT_BITS   (64 - FRACTION_BITS - 1)
#define EXPONENT_MASK   (((uint64_t) 1 << EXPONENT_BITS) - 1)
#define SIGN_BIT        ((uint64_t) 1 << (64 - 1))
#define BIT64(x)        ((uint64_t) 1 << (x))

/*
 * Check to see if the high bit is set by casting
 * to signed and checking for negative
 */
static inline bool
high_bit_set(uint64_t fract)
{
        return ((int64_t) fract < 0);
}

int
__dtoa_engine(FLOAT64 x, struct dtoa *dtoa, int max_digits, bool fmode, int max_decimals)
{
        uint64_t v = asuint64(x);
        uint64_t fract = (v << (EXPONENT_BITS + 1)) >> 1;
        int expo = (v << 1) >> (FRACTION_BITS + 1);
        int decexp = 0;
        int i;
	uint8_t	flags = 0;

	if (high_bit_set(v))
		flags |= DTOA_MINUS;
	if (expo == EXPONENT_MASK) {
                if (fract)
                        flags |= DTOA_NAN;
                else
                        flags |= DTOA_INF;
        } else {
                if (expo == 0) {
                        if (fract == 0) {
                                flags |= DTOA_ZERO;
                                dtoa->digits[0] = '0';
                                max_digits = 1;
                                goto done;
                        }
                        /* normalize */
                        while ((int64_t) (fract <<= 1) >= 0)
                                expo--;
                }

                /* add implicit bit */
                fract |= SIGN_BIT;
                /* adjust exponent */
                expo -= (DTOA_MAX_EXP - 2);
                decexp = -1;

                /*
                 * Let's consider:
                 *
                 *	value = fract * 2^expo * 10^decexp
                 *
                 * Initially decexp = 0. The goal is to bring exp between
                 * 0 and -2 as the magnitude of a fractional decimal digit is 3 bits.
                 */

                while (expo < -2) {
                        /*
                         * Make room to allow a multiplication by 5 without overflow.
                         * We test only the top part for faster code.
                         */
                        do {
                                /* Round this division to avoid accumulating errors */
                                fract = (fract >> 1) + (fract&1);
                                expo++;
                        } while ((uint32_t)(fract >> 32) >= (UINT32_MAX / 5U));

                        /* Perform fract * 5 * 2 / 10 */
                        fract *= 5U;
                        expo++;
                        decexp--;
                }

                while (expo > 0) {
                        /*
                         * Perform fract / 5 / 2 * 10.
                         * The +2 is there to do round the result of the division
                         * by 5 not to lose too much precision in extreme cases.
                         */
                        fract += 2;
                        fract /= 5U;
                        expo--;
                        decexp++;

                        /* Bring back our fractional number to full scale */
                        do {
                                fract <<= 1;
                                expo--;
                        } while (!high_bit_set(fract));
                }


                /*
                 * The binary fractional point is located somewhere above bit 63.
                 * Move it between bits 59 and 60 to give 4 bits of room to the
                 * integer part.
                 */
                fract >>= (4 - expo);

		/* If limiting decimals, then limit the max digits
		 * to no more than the number of digits left of the decimal
		 * plus the number of digits right of the decimal
		 *
		 * exp:          exponent value. If negative, there are
		 *		 -exp - 1 zeros left of the first non-zero
		 *               digit in 'f' format. If non-negative,
		 *               there are exp digits to the left of
		 *               the decimal point
		 *
		 * max_decimals: Only used in 'f' format. Round to no
		 *               more than this many digits to the
		 *               right of the decimal point (left if
		 *               negative)
		 *
		 * max_digits: We can't convert more than this number
		 *               of digits given the limits of the
		 *               buffer
		 */

		int save_max_digits = max_digits;
		if (fmode) {
			/*
			 * This covers two cases:
			 *
			 * When exp is < 0, there are -exp-1 zeros taking up
			 * space before we can display any of the real digits,
			 * so we have to subtract those off max_decimals before
			 * we round that (max_decimals - (-exp - 1)). This
			 * may end up less than zero, in which case we have
			 * no digits to display.
			 *
			 * When exp >= 0, there are exp + 1 digits left of the
			 * decimal point *plus* max_decimals right of the
			 * decimal point that need to be generated
			 *
			 * A single expression gives the right answer in both
			 * cases, which is kinda cool
			 */
			/* max_decimals comes in biased by 1 to flag the 'f' case */
			max_digits = min(max_digits, max(1, max_decimals + decexp + 1));
		}

                int decimals = max_digits;

                /* Round the value to the last digit being printed. */
                uint64_t round = BIT64(59); /* 0.5 */
                while (decimals--) {
                        round /= 10U;
                }
                fract += round;
                /* Make sure rounding didn't make fract >= 1.0 */
                if (fract >= BIT64(60)) {
                        fract /= 10U;
                        decexp++;
                        max_digits = min(save_max_digits, max(1, max_decimals + decexp + 1));
                }

		/* Now convert mantissa to decimal. */

		/* Compute digits */
		for (i = 0; i < max_digits; i++) {
                        fract *= 10U;
			dtoa->digits[i] = (fract >> 60) + '0';
                        fract &= BIT64(60) - 1;
		}
	}
done:
	dtoa->flags = flags;
	dtoa->exp = decexp;
	return max_digits;
}

#endif
