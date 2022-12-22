/* sf_finite.c -- float version of s_finite.c.
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

/*
 * finitef(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include "fdlibm.h"

int finitef(float x)
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	return (FLT_UWORD_IS_FINITE(ix));
}

#if defined(_HAVE_ALIAS_ATTRIBUTE)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(finitef, __finitef);
#else

int
__finitef(float x)
{
    return finitef(x);
}
#endif

_MATH_ALIAS_i_f(finite)
_MATH_ALIAS_i_f(__finite)
