/*
 * isinff(x) returns 1 if x is +-infinity, else 0;
 *
 * isinf is a <math.h> macro in the C99 standard.  It was previously
 * implemented as isinf and isinff functions by newlib and are still declared
 * as such in <math.h>.  Newlib supplies it here as a function if the user
 * chooses to use it instead of the C99 macro.
 */

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

#ifdef _DOUBLE_IS_32BITS

#undef isinf

int
isinf (double x)
{
	return isinff((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
