
/* @(#)s_isnan.c 5.1 93/09/24 */
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
<<fpclassify>>, <<isfinite>>, <<isinf>>, <<isnan>>, and <<isnormal>>---floating-point classification macros; <<finite>>, <<finitef>>, <<isinf>>, <<isinff>>, <<isnan>>, <<isnanf>>---test for exceptional numbers

@c C99 (start
INDEX
	fpclassify
INDEX
	isfinite
INDEX
	isinf
INDEX
	isnan
INDEX
	isnormal
@c C99 end)
@c SUSv2 (start
INDEX
	isnan
INDEX
	isinf
INDEX
	finite

INDEX
	isnanf
INDEX
	isinff
INDEX
	finitef
@c SUSv2 end)

ANSI_SYNOPSIS
	[C99 standard macros:]
	#include <math.h>
	int fpclassify(real-floating <[x]>);
	int isfinite(real-floating <[x]>);
	int isinf(real-floating <[x]>);
	int isnan(real-floating <[x]>);
	int isnormal(real-floating <[x]>);

	[Archaic SUSv2 functions:]
	#include <math.h>
	int isnan(double <[arg]>);
	int isinf(double <[arg]>);
	int finite(double <[arg]>);
	int isnanf(float <[arg]>);
	int isinff(float <[arg]>);
	int finitef(float <[arg]>);

DESCRIPTION
<<fpclassify>>, <<isfinite>>, <<isinf>>, <<isnan>>, and <<isnormal>> are macros
defined for use in classifying floating-point numbers.  This is a help because
of special "values" like NaN and infinities.  In the synopses shown,
"real-floating" indicates that the argument is an expression of real floating
type.  These function-like macros are C99 and POSIX-compliant, and should be
used instead of the now-archaic SUSv2 functions.

The <<fpclassify>> macro classifies its argument value as NaN, infinite, normal,
subnormal, zero, or into another implementation-defined category.  First, an
argument represented in a format wider than its semantic type is converted to
its semantic type.  Then classification is based on the type of the argument.
The <<fpclassify>> macro returns the value of the number classification macro
appropriate to the value of its argument:

o+
o FP_INFINITE
	<[x]> is either plus or minus infinity;
o FP_NAN
	<[x]> is "Not A Number" (plus or minus);
o FP_NORMAL
	<[x]> is a "normal" number (i.e. is none of the other special forms);
o FP_SUBNORMAL
	<[x]> is too small be stored as a regular normalized number (i.e. loss of precision is likely); or
o FP_ZERO
	<[x]> is 0 (either plus or minus).
o-

The "<<is>>" set of macros provide a useful set of shorthand ways for
classifying floating-point numbers, providing the following equivalent
relations:

o+
o <<isfinite>>(<[x]>)
returns non-zero if <[x]> is finite.  (It is equivalent to
(<<fpclassify>>(<[x]>) != FP_INFINITE  &&  <<fpclassify>>(<[x]>) != FP_NAN).)

o <<isinf>>(<[x]>)
returns non-zero if <[x]> is infinite.  (It is equivalent to
(<<fpclassify>>(<[x]>) == FP_INFINITE).)

o <<isnan>>(<[x]>)
returns non-zero if <[x]> is NaN.  (It is equivalent to
(<<fpclassify>>(<[x]>) == FP_NAN).)

o <<isnormal>>(<[x]>)
returns non-zero if <[x]> is normal.  (It is equivalent to
(<<fpclassify>>(<[x]>) == FP_NORMAL).)
o-

	The archaic SUSv2 functions provide information on the floating-point
	argument supplied.

	There are five major number formats ("exponent" referring to the
	biased exponent in the binary-encoded number):
	o+
	o zero
	  A number which contains all zero bits, excluding the sign bit.
	o subnormal
	  A number with a zero exponent but a nonzero fraction.
	o normal
	  A number with an exponent and a fraction.
     	o infinity
	  A number with an all 1's exponent and a zero fraction.
	o NAN
	  A number with an all 1's exponent and a nonzero fraction.

	o-

	<<isnan>> returns 1 if the argument is a nan. <<isinf>>
	returns 1 if the argument is infinity.  <<finite>> returns 1 if the
	argument is zero, subnormal or normal.
	
	The <<isnanf>>, <<isinff>> and <<finitef>> functions perform the same
	operations as their <<isnan>>, <<isinf>> and <<finite>>
	counterparts, but on single-precision floating-point numbers.

	It should be noted that the C99 standard dictates that <<isnan>>
	and <<isinf>> are macros that operate on multiple types of
	floating-point.  The SUSv2 standard declares <<isnan>> as
	a function taking double.  Newlib has decided to declare
	them both as functions and as macros in math.h to
	maintain backward compatibility.

RETURNS
@comment Formatting note:  "$@" forces a new line
The fpclassify macro returns the value corresponding to the appropriate FP_ macro.@*
The isfinite macro returns nonzero if <[x]> is finite, else 0.@*
The isinf macro returns nonzero if <[x]> is infinite, else 0.@*
The isnan macro returns nonzero if <[x]> is an NaN, else 0.@*
The isnormal macro returns nonzero if <[x]> has a normal value, else 0.

PORTABILITY
math.h macros are C99, POSIX.1-2001.

The functions originate from BSD; isnan was listed in the X/Open
Portability Guide and Single Unix Specification, but was dropped when
the macro was standardized in POSIX.1-2001.

QUICKREF
	isnan - pure
QUICKREF
	isinf - pure
QUICKREF
	finite - pure
QUICKREF
	isnan - pure
QUICKREF
	isinf - pure
QUICKREF
	finite - pure
*/

/*
 * isnan(x) returns 1 is x is nan, else 0;
 * no branching!
 *
 * The C99 standard dictates that isnan is a macro taking
 * multiple floating-point types while the SUSv2 standard
 * notes it is a function taking a double argument.  Newlib
 * has chosen to declare it both as a function and as a macro in
 * <math.h> for compatibility.
 */

#include "fdlibm.h"
#include <ieeefp.h>

#ifndef _DOUBLE_IS_32BITS

#undef isnan

#ifdef __STDC__
	int isnan(double x)
#else
	int isnan(x)
	double x;
#endif
{
	__int32_t hx,lx;
	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;
	hx |= (__uint32_t)(lx|(-lx))>>31;	
	hx = 0x7ff00000 - hx;
	return (int)(((__uint32_t)(hx))>>31);
}

#endif /* _DOUBLE_IS_32BITS */
