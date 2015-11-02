/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/*
FUNCTION
<<fmax>>, <<fmaxf>>---maximum
INDEX
	fmax
INDEX
	fmaxf

ANSI_SYNOPSIS
	#include <math.h>
	double fmax(double <[x]>, double <[y]>);
	float fmaxf(float <[x]>, float <[y]>);

DESCRIPTION
The <<fmax>> functions determine the maximum numeric value of their arguments.
NaN arguments are treated as missing data:  if one argument is a NaN and the
other numeric, then the <<fmax>> functions choose the numeric value.

RETURNS
The <<fmax>> functions return the maximum numeric value of their arguments.

PORTABILITY
ANSI C, POSIX.

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double fmax(double x, double y)
#else
	double fmax(x,y)
	double x;
	double y;
#endif
{
  if (__fpclassifyd(x) == FP_NAN)
    return y;
  if (__fpclassifyd(y) == FP_NAN)
    return x;
  
  return x > y ? x : y;
}

#endif /* _DOUBLE_IS_32BITS */
