/*
 * isinff(x) returns 1 if x is +-infinity, else 0;
 * Added by Cygnus Support.
 */

#include "fdlibm.h"

#ifdef __STDC__
	int isinff(float x)
#else
	int isinff(x)
	float x;
#endif
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	return FLT_UWORD_IS_INFINITE(ix);
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	int isinf(double x)
#else
	int isinf(x)
	double x;
#endif
{
	return isinff((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
