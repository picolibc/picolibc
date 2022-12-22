/* sf_c_isnan.c -- float version of s_c_isnan.c.
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

/*
 * isnanf(x) returns 1 is x is nan, else 0;
 *
 * isnanf is an extension declared in <math.h>.
 */

#include "fdlibm.h"
#include <ieeefp.h>
 
#undef isnanf

int
isnanf (float x)
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	return FLT_UWORD_IS_NAN(ix);
}

#undef isnan

_MATH_ALIAS_i_f(isnan)
