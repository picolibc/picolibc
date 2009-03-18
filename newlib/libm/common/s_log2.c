/* @(#)s_log2.c 5.1 93/09/24 */
/* Modification from s_exp10.c Yaakov Selkowitz 2009.  */

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
FUNCTION
	<<log2>>, <<log2f>>---base 2 logarithm
INDEX
	log2
INDEX
	log2f

ANSI_SYNOPSIS
	#include <math.h>
	double log2(double <[x]>);
	float log2f(float <[x]>);

TRAD_SYNOPSIS
	#include <math.h>
	double log2(<[x]>);
	double <[x]>;

	float log2f(<[x]>);
	float <[x]>;

DESCRIPTION
	<<log2>> returns the base 2 logarithm of <[x]>.

	<<log2f>> is identical, save that it takes and returns <<float>> values.

RETURNS
	On success, <<log2>> and <<log2f>> return the calculated value.
	If the result underflows, the returned value is <<0>>.  If the
	result overflows, the returned value is <<HUGE_VAL>>.  In
	either case, <<errno>> is set to <<ERANGE>>.

PORTABILITY
	<<log2>> and <<log2f>> are required by ISO/IEC 9899:1999 and the
	System V Interface Definition (Issue 6).
*/

/*
 * wrapper log2(x)
 */

#include "fdlibm.h"
#include <errno.h>
#include <math.h>
#undef log2

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log2(double x)		/* wrapper log2 */
#else
	double log2(x)			/* wrapper log2 */
	double x;
#endif
{
  return (log(x) / M_LOG2_E);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
