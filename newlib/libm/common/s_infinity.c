/*
 * infinity () returns the representation of infinity.
 * Added by Cygnus Support.
 */

/*
FUNCTION
	<<infinity>>, <<infinityf>>--representation of infinity

INDEX
	infinity
INDEX
	infinityf

ANSI_SYNOPSIS
	#include <math.h>
	double infinity(void);
	float infinityf(void);

DESCRIPTION
	<<infinity>> and <<infinityf>> return the special number IEEE
	infinity in double- and single-precision arithmetic
	respectively.

PORTABILITY
<<infinity>> and <<infinityf>> are neither standard C nor POSIX.  C and
POSIX require macros HUGE_VAL and HUGE_VALF to be defined in math.h, which
Newlib defines to be infinities corresponding to these archaic infinity()
and infinityf() functions in floating-point implementations which do have
infinities.

QUICKREF
	infinity - pure

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

	double infinity()
{
	double x;

	INSERT_WORDS(x,0x7ff00000,0);
	return x;
}

#endif /* _DOUBLE_IS_32BITS */
