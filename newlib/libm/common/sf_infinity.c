/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * infinityf () returns the representation of infinity.
 * Added by Cygnus Support.
 */

#include "fdlibm.h"

float infinityf(void)
{
	float x;

	SET_FLOAT_WORD(x,0x7f800000);
	return x;
}

#ifdef _DOUBLE_IS_32BITS

double infinity(void)
{
	return (double) infinityf();
}

#endif /* defined(_DOUBLE_IS_32BITS) */
