/*							coshl.c
 *
 *	Hyperbolic cosine, long double precision
 *
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, coshl();
 *
 * y = coshl( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns hyperbolic cosine of argument in the range MINLOGL to
 * MAXLOGL.
 *
 * cosh(x)  =  ( exp(x) + exp(-x) )/2.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE     +-10000      30000       1.1e-19     2.8e-20
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition              value returned
 * cosh overflow    |x| > MAXLOGL+LOGE2L      INFINITYL
 *
 *
 */


/*
Cephes Math Library Release 2.7:  May, 1998
Copyright 1985, 1991, 1998 by Stephen L. Moshier
*/

/*
Modified for mingw
2002-07-22 Danny Smith <dannysmith@users.sourceforge.net>
*/

#ifdef __MINGW32__
#include "cephes_mconf.h"
#else
#include "mconf.h"
#endif

#ifndef _SET_ERRNO
#define _SET_ERRNO(x)
#endif


#ifndef __MINGW32__
extern long double MAXLOGL, MAXNUML, LOGE2L;
#ifdef ANSIPROT
extern long double expl ( long double );
extern int isnanl ( long double );
#else
long double expl(), isnanl();
#endif
#ifdef INFINITIES
extern long double INFINITYL;
#endif
#ifdef NANS
extern long double NANL;
#endif
#endif /* __MINGW32__ */

long double coshl(x)
long double x;
{
long double y;

#ifdef NANS
if( isnanl(x) )
	{
	_SET_ERRNO(EDOM);
	return(x);
  	}
#endif
if( x < 0 )
	x = -x;
if( x > (MAXLOGL + LOGE2L) )
	{
	mtherr( "coshl", OVERFLOW );
	_SET_ERRNO(ERANGE);
#ifdef INFINITIES
	return( INFINITYL );
#else
	return( MAXNUML );
#endif
	}	
if( x >= (MAXLOGL - LOGE2L) )
	{
	y = expl(0.5L * x);
	y = (0.5L * y) * y;
	return(y);
	}
y = expl(x);
y = 0.5L * (y + 1.0L / y);
return( y );
}
