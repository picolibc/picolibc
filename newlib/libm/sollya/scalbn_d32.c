/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#define WANT_FLOAT32

#include "math2.h"

#if INT_MAX > 50000
#define OVERFLOW_INT 50000
#else
#define OVERFLOW_INT 30000
#endif

float
name(_ff_scalbn)(ff_t xx, int n)
{
        float_t x = xx.hi;
	fint_t k,ix;
	fuint_t hx;

        ix = (fint_t) name(touint)(x);
	hx = ix&0x7fffffff;
        k = hx>>23;		/* extract exponent */
        if (k == 0) {
            if (hx == 0) return x;
	    x *= 0x1p23f;
            ix = (fint_t) name(touint)(x);
	    k = ((ix&0x7f800000)>>23) - 23; 
#if __SIZEOF_INT__ > 2
            if (n< -50000)
                return __math_uflowf(ix<0); 	/*underflow*/
#endif
        }
        if (k == 0xff)  return x + x;	/* NaN or Inf */
        if (n > OVERFLOW_INT) 	/* in case integer overflow in n+k */
            return __math_oflowf(ix<0);	        /*overflow*/
        k = k+n; 
        if (k > FLT_LARGEST_EXP)
            return __math_oflowf(ix<0);          /* overflow  */
        if (k > 0) 				/* normal result */
	    {SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23)); return x;}
        if (k <= -24)
	    return __math_uflowf(ix<0);	        /*underflow*/
        k += 24;				/* subnormal result */

        ix = (ix&0x807fffff)|(k<<23);

        if (n < 0 && xx.lo != 0) {
            fint_t ixl = asr((fint_t) name(touint)(xx.lo) ^ ix, 31) | 1;

            if (k == 24) {
                if (ixl >= 0) {
                    /* when signs are the same, if the new low bit is zero, add one */
                    if ((ix & 2) == 0)
                        ix++;
                } else {
                    /* when the signs differ, clear the bottom bit */
                    ix &= ~1;
                }
            } else {
                if ((ix & 1) == 0)
                    ix += ixl;
            }
        }

	SET_FLOAT_WORD(x,ix);
        return check_uflowf(x*0x1p-24f);
}
