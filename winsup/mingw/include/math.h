/* 
 * math.h
 *
 * Mathematical functions.
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#ifndef _MATH_H_
#define _MATH_H_

/* All the headers include this file. */
#include <_mingw.h>

/*
 * Types for the _exception structure.
 */

#define	_DOMAIN		1	/* domain error in argument */
#define	_SING		2	/* singularity */
#define	_OVERFLOW	3	/* range overflow */
#define	_UNDERFLOW	4	/* range underflow */
#define	_TLOSS		5	/* total loss of precision */
#define	_PLOSS		6	/* partial loss of precision */

/*
 * Exception types with non-ANSI names for compatibility.
 */

#ifndef	__STRICT_ANSI__
#ifndef	_NO_OLDNAMES

#define	DOMAIN		_DOMAIN
#define	SING		_SING
#define	OVERFLOW	_OVERFLOW
#define	UNDERFLOW	_UNDERFLOW
#define	TLOSS		_TLOSS
#define	PLOSS		_PLOSS

#endif	/* Not _NO_OLDNAMES */
#endif	/* Not __STRICT_ANSI__ */


/* These are also defined in Mingw float.h; needed here as well to work 
   around GCC build issues.  */
#ifndef	__STRICT_ANSI__
#ifndef __MINGW_FPCLASS_DEFINED
#define __MINGW_FPCLASS_DEFINED 1
#define	_FPCLASS_SNAN	0x0001	/* Signaling "Not a Number" */
#define	_FPCLASS_QNAN	0x0002	/* Quiet "Not a Number" */
#define	_FPCLASS_NINF	0x0004	/* Negative Infinity */
#define	_FPCLASS_NN	0x0008	/* Negative Normal */
#define	_FPCLASS_ND	0x0010	/* Negative Denormal */
#define	_FPCLASS_NZ	0x0020	/* Negative Zero */
#define	_FPCLASS_PZ	0x0040	/* Positive Zero */
#define	_FPCLASS_PD	0x0080	/* Positive Denormal */
#define	_FPCLASS_PN	0x0100	/* Positive Normal */
#define	_FPCLASS_PINF	0x0200	/* Positive Infinity */
#endif /* __MINGW_FPCLASS_DEFINED */
#endif	/* Not __STRICT_ANSI__ */

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

/*
 * HUGE_VAL is returned by strtod when the value would overflow the
 * representation of 'double'. There are other uses as well.
 *
 * __imp__HUGE is a pointer to the actual variable _HUGE in
 * MSVCRT.DLL. If we used _HUGE directly we would get a pointer
 * to a thunk function.
 *
 * NOTE: The CRTDLL version uses _HUGE_dll instead.
 */

#ifndef __DECLSPEC_SUPPORTED

#ifdef __MSVCRT__
extern double*	__imp__HUGE;
#define	HUGE_VAL	(*__imp__HUGE)
#else
/* CRTDLL */
extern double*	__imp__HUGE_dll;
#define	HUGE_VAL	(*__imp__HUGE_dll)
#endif

#else /* __DECLSPEC_SUPPORTED */

#ifdef __MSVCRT__
__MINGW_IMPORT double	_HUGE;
#define	HUGE_VAL	_HUGE
#else
/* CRTDLL */
__MINGW_IMPORT double	_HUGE_dll;
#define	HUGE_VAL	_HUGE_dll
#endif

#endif /* __DECLSPEC_SUPPORTED */

struct _exception
{
	int	type;
	char	*name;
	double	arg1;
	double	arg2;
	double	retval;
};


double	sin (double);
double	cos (double);
double	tan (double);
double	sinh (double);
double	cosh (double);
double	tanh (double);
double	asin (double);
double	acos (double);
double	atan (double);
double	atan2 (double, double);
double	exp (double);
double	log (double);
double	log10 (double);
double	pow (double, double);
double	sqrt (double);
double	ceil (double);
double	floor (double);
double	fabs (double);
double	ldexp (double, int);
double	frexp (double, int*);
double	modf (double, double*);
double	fmod (double, double);


#ifndef	__STRICT_ANSI__

/* Complex number (for cabs) */
struct _complex
{
	double	x;	/* Real part */
	double	y;	/* Imaginary part */
};

double	_cabs (struct _complex);
double	_hypot (double, double);
double	_j0 (double);
double	_j1 (double);
double	_jn (int, double);
double	_y0 (double);
double	_y1 (double);
double	_yn (int, double);
int	_matherr (struct _exception *);

/* These are also declared in Mingw float.h; needed here as well to work 
   around GCC build issues.  */
/* BEGIN FLOAT.H COPY */
/*
 * IEEE recommended functions
 */

double	_chgsign	(double);
double	_copysign	(double, double);
double	_logb		(double);
double	_nextafter	(double, double);
double	_scalb		(double, long);

int	_finite		(double);
int	_fpclass	(double);
int	_isnan		(double);

/* END FLOAT.H COPY */

#ifndef	_NO_OLDNAMES

/*
 * Non-underscored versions of non-ANSI functions. These reside in
 * liboldnames.a. Provided for extra portability.
 */
double cabs (struct _complex);
double hypot (double, double);
double j0 (double);
double j1 (double);
double jn (int, double);
double y0 (double);
double y1 (double);
double yn (int, double);

#endif	/* Not _NO_OLDNAMES */

#endif	/* Not __STRICT_ANSI__ */

#ifdef __cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _MATH_H_ */

