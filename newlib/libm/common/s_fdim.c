/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/*
FUNCTION
<<fdim>>, <<fdimf>>---positive difference
INDEX
	fdim
INDEX
	fdimf

ANSI_SYNOPSIS
	#include <math.h>
	double fdim(double <[x]>, double <[y]>);
	float fdimf(float <[x]>, float <[y]>);

DESCRIPTION
The <<fdim>> functions determine the positive difference between their
arguments, returning:
.	<[x]> - <[y]>	if <[x]> > <[y]>, or
	@ifnottex
.	+0	if <[x]> <= <[y]>, or
	@end ifnottex
	@tex
.	+0	if <[x]> $\leq$ <[y]>, or
	@end tex
.	NAN	if either argument is NAN.
A range error may occur.

RETURNS
The <<fdim>> functions return the positive difference value.

PORTABILITY
ANSI C, POSIX.

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double fdim(double x, double y)
#else
	double fdim(x,y)
	double x;
	double y;
#endif
{
  int c = __fpclassifyd(x);
  if (c == FP_NAN)  return(x);
  if (__fpclassifyd(y) == FP_NAN)  return(y);
  if (c == FP_INFINITE)
    return HUGE_VAL;

  return x > y ? x - y : 0.0;
}

#endif /* _DOUBLE_IS_32BITS */
