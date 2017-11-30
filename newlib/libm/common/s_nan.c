/*
 * nan () returns a nan.
 * Added by Cygnus Support.
 */

/*
FUNCTION
	<<nan>>, <<nanf>>---representation of ``Not a Number''

INDEX
	nan
INDEX
	nanf

SYNOPSIS
	#include <math.h>
	double nan(const char *<[unused]>);
	float nanf(const char *<[unused]>);


DESCRIPTION
	<<nan>> and <<nanf>> return an IEEE NaN (Not a Number) in
	double- and single-precision arithmetic respectively.  The
	argument is currently disregarded.

QUICKREF
	nan - pure

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

	double nan(const char *unused)
{
	double x;

#if __GNUC_PREREQ (3, 3)
	x = __builtin_nan("");
#else
	INSERT_WORDS(x,0x7ff80000,0);
#endif
	return x;
}

#endif /* _DOUBLE_IS_32BITS */
