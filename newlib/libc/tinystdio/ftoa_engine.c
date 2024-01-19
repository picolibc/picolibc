/* Copyright (c) 2005, Dmitry Xmelkov
   All rights reserved.
   Rewritten in C by Soren Kuula
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

#define _NEED_IO_FLOAT32
#include "dtoa.h"

/*
 * 2^b ~= f * r * 10^e
 * where
 * i = b div 8
 * r = 2^(b mod 8)
 * f = factorTable[i]
 * e = exponentTable[i]
 */
static const int8_t exponentTable[32] = {
    -36, -33, -31, -29, -26, -24, -21, -19,
    -17, -14, -12, -9,  -7, -4, -2,  0,
    3, 5, 8, 10, 12, 15,  17, 20,
    22, 24, 27, 29,  32, 34, 36, 39
};

static const uint32_t factorTable[32] = {
    2295887404UL,
    587747175UL,
    1504632769UL,
    3851859889UL,
    986076132UL,
    2524354897UL,
    646234854UL,
    1654361225UL,
    4235164736UL,
    1084202172UL,
    2775557562UL,
    710542736UL,
    1818989404UL,
    465661287UL,
    1192092896UL,
    3051757813UL,
    781250000UL,
    2000000000UL,
    512000000UL,
    1310720000UL,
    3355443200UL,
    858993459UL,
    2199023256UL,
    562949953UL,
    1441151881UL,
    3689348815UL,
    944473297UL,
    2417851639UL,
    618970020UL,
    1584563250UL,
    4056481921UL,
    1038459372UL
};

#define max(a, b) ({\
		__typeof(a) _a = a;\
		__typeof(b) _b = b;\
		_a > _b ? _a : _b; })

#define min(a, b) ({\
		__typeof(a) _a = a;\
		__typeof(b) _b = b;\
		_a < _b ? _a : _b; })

int __ftoa_engine(float val, struct dtoa *ftoa, int maxDigits, bool fmode, int maxDecimals) 
{
    uint8_t flags = 0;

    union {
        float v;
        uint32_t u;
    } x;

    x.v = val;

    uint32_t frac = x.u & 0x007fffffUL;

    if (maxDigits > FTOA_MAX_DIG)
        maxDigits = FTOA_MAX_DIG;

    /* Read the sign, shift the exponent in place and delete it from frac.
     */
    if (x.u & ((uint32_t)1 << 31))
        flags = DTOA_MINUS;

    uint8_t exp = x.u >> 23;

    ftoa->exp = 0;

    /*
     * Test for easy cases, zero and NaN
     */
    if(exp==0 && frac==0)
    {
        flags |= DTOA_ZERO;
	ftoa->digits[0] = '0';
	maxDigits = 1;
    } else if(exp == 0xff) {
        if(frac == 0)
            flags |= DTOA_INF;
        else
            flags |= DTOA_NAN;
    } else {

        /* The implicit leading 1 is made explicit, except if value
         * subnormal, in which case exp needs to be adjusted by one
         */
        if (exp == 0)
	    exp = 1;
	else
            frac |= (1UL<<23);

        uint8_t idx = exp>>3;
        int8_t exp10 = exponentTable[idx];

        /*
         * We COULD try making the multiplication in situ, where we make
         * frac and a 64 bit int overlap in memory and select/weigh the
         * upper 32 bits that way. For starters, this is less risky:
         */
        int64_t prod = (int64_t)frac * (int64_t)factorTable[idx];

        /*
         * The expConvFactorTable are factor are correct iff the lower 3 exponent
         * bits are 1 (=7). Else we need to compensate by divding frac.
         * If the lower 3 bits are 7 we are right.
         * If the lower 3 bits are 6 we right-shift once
         * ..
         * If the lower 3 bits are 0 we right-shift 7x
         */
        prod >>= (15-(exp & 7));

        /*
         * Now convert to decimal.
         */

        uint8_t hadNonzeroDigit = 0; /* have seen a non-zero digit flag */
        uint8_t outputIdx = 0;
        int64_t decimal = 100000000000000ull;
	uint8_t saveMaxDigits = maxDigits;

        do {
            /* Compute next digit */
            char digit = prod / decimal + '0';

            if(!hadNonzeroDigit)
            {
                /* Don't return results with a leading zero! Instead
                 * skip those and decrement exp10 accordingly.
                 */
                if (digit == '0') {
                    exp10--;
		    prod = prod % decimal;
		    decimal /= 10;
                    continue;
                }

                /* Found the first non-zero digit */
                hadNonzeroDigit = 1;

                /* If limiting decimals... */
                if(fmode)
                {
		    maxDigits = min(maxDigits, max(1, maxDecimals + exp10 + 1));
		    if (maxDigits == 0)
			break;
                }
            }
            prod = prod % decimal;
            decimal /= 10;

            /* Now we have a digit. */
            if(digit < '0' + 10) {
                /* normal case. */
                ftoa->digits[outputIdx] = digit;
            } else {
                /*
                 * Uh, oh. Something went wrong with our conversion. Write
                 * 9s and use the round-up code to report back to the caller
                 * that we've carried
                 */
                for(outputIdx = 0; outputIdx < maxDigits; outputIdx++)
                    ftoa->digits[outputIdx] = '9';
                goto round_up;
            }
            outputIdx++;
        } while (outputIdx<maxDigits);

        /* Rounding: */
        decimal *= 10;
        if (prod - (decimal >> 1) >= 0)
        {
	    uint8_t rounded;
        round_up:

	    rounded = 0;
            while(outputIdx != 0) {

                /* Increment digit, check if we're done */
                if(++ftoa->digits[outputIdx-1] < '0' + 10) {
		    rounded = 1;
                    break;
		}

                ftoa->digits[--outputIdx] = '0';
                /* and the loop continues, carrying to next digit. */
	    }

	    if (!rounded) {

                /* Rounded past the first digit;
                 * reset the leading digit to 1,
                 * bump exp and tell the caller we've carried.
                 * The remaining digits will already be '0',
                 * so we don't need to mess with them
                 */
		ftoa->digits[0] = '1';
		exp10++;
		if (fmode)
		    maxDigits = min(saveMaxDigits, max(1, maxDecimals + exp10 + 1));
		flags |= DTOA_CARRY;
	    }
        }
        ftoa->exp = exp10;
    }
    ftoa->flags = flags;
    return maxDigits;
}
