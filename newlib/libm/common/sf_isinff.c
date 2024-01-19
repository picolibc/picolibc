/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * __isinff(x) returns 1 if x is +-infinity, else 0;
 * Added by Cygnus Support.
 */

#include "fdlibm.h"

int
__isinff (float x)
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	return FLT_UWORD_IS_INFINITE(ix);
}

_MATH_ALIAS_i_f(__isinf)
