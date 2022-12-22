/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * isinf(x) returns 1 if x is infinity, else 0;
 * no branching!
 *
 * isinf is a <math.h> macro in the C99 standard.  It was previously
 * implemented as a function by newlib and is declared as such in
 * <math.h>.  Newlib supplies it here as a function if the user
 * chooses to use it instead of the C99 macro.
 */

#include "fdlibm.h"
#include <ieeefp.h>

#ifdef _NEED_FLOAT64

#undef isinf

int
isinf64(__float64 x)
{
	__int32_t hx,lx;
	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;
	hx |= (__uint32_t)(lx|(-lx))>>31;	
	hx = 0x7ff00000 - hx;
	return 1 - (int)((__uint32_t)(hx|(-hx))>>31);
}

_MATH_ALIAS_i_d(isinf)

#endif /* _NEED_FLOAT64 */
