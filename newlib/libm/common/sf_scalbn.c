/* sf_scalbn.c -- float version of s_scalbn.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include "fdlibm.h"
#include <limits.h>
#include <float.h>

#if INT_MAX > 50000
#define OVERFLOW_INT 50000
#else
#define OVERFLOW_INT 30000
#endif

static const float
two25   =  3.355443200e+07,	/* 0x4c000000 */
twom25  =  2.9802322388e-08;	/* 0x33000000 */

float scalbnf (float x, int n)
{
	__int32_t  k,ix;
	__uint32_t hx;

	GET_FLOAT_WORD(ix,x);
	hx = ix&0x7fffffff;
        k = hx>>23;		/* extract exponent */
        if (k == 0) {
            if (hx == 0) return x;
	    x *= two25;
	    GET_FLOAT_WORD(ix,x);
	    k = ((ix&0x7f800000)>>23) - 25; 
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
        if (k < -25)
	    return __math_uflowf(ix<0);	        /*underflow*/
        k += 25;				/* subnormal result */
	SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23));
        return check_uflowf(x*twom25);
}

#if defined(_HAVE_ALIAS_ATTRIBUTE)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(scalbnf, ldexpf);
#else

float
ldexpf(float value, int exp)
{
    return scalbnf(value, exp);
}

#endif

_MATH_ALIAS_f_fi(scalbn)
_MATH_ALIAS_f_fi(ldexp)
