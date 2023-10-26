/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * isinff(x) returns 1 if x is +-infinity, else 0;
 *
 * isinf is a <math.h> macro in the C99 standard.  It was previously
 * implemented as isinf and isinff functions by newlib and are still declared
 * as such in <math.h>.  Newlib supplies it here as a function if the user
 * chooses to use it instead of the C99 macro.
 */

#define _ADD_D_TO_DOUBLE_FUNCS

#include "fdlibm.h"
#include <ieeefp.h>

#undef isinff

int
isinff (float x)
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	return FLT_UWORD_IS_INFINITE(ix);
}

#undef isinf

_MATH_ALIAS_i_f(isinf)
#ifdef _DOUBLE_IS_32BITS
#define isinfd __isinfd
_MATH_ALIAS_i_d_to_f(isinf)
#endif
