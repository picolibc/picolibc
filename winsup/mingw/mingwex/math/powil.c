/*							__powil.c
 *
 *	Real raised to integer power, long double precision
 *
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, __powil();
 * int n;
 *
 * y = __powil( x, n );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns argument x raised to the nth power.
 * The routine efficiently decomposes n as a sum of powers of
 * two. The desired power is a product of two-to-the-kth
 * powers of x.  Thus to compute the 32767 power of x requires
 * 28 multiplications instead of 32767 multiplications.
 *
 *
 *
 * ACCURACY:
 *
 *
 *                      Relative error:
 * arithmetic   x domain   n domain  # trials      peak         rms
 *    IEEE     .001,1000  -1022,1023  50000       4.3e-17     7.8e-18
 *    IEEE        1,2     -1022,1023  20000       3.9e-17     7.6e-18
 *    IEEE     .99,1.01     0,8700    10000       3.6e-16     7.2e-17
 *
 * Returns INFINITY on overflow, zero on underflow.
 *
 */

/*							__powil.c	*/

/*
Cephes Math Library Release 2.2:  December, 1990
Copyright 1984, 1990 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/*
Modified for mingw
2002-07-22 Danny Smith <dannysmith@users.sourceforge.net>
*/

#ifdef __MINGW32__
#include "cephes_mconf.h"
#else
#include "mconf.h"
extern long double MAXNUML, MAXLOGL, MINLOGL;
extern long double LOGE2L;
#ifdef ANSIPROT
extern long double frexpl ( long double, int * );
#else
long double frexpl();
#endif
#endif /* __MINGW32__ */

#ifndef _SET_ERRNO
#define _SET_ERRNO(x)
#endif

long double __powil( x, nn )
long double x;
int nn;
{
long double w, y;
long double s;
int n, e, sign, asign, lx;

if( x == 0.0L )
	{
	if( nn == 0 )
		return( 1.0L );
	else if( nn < 0 )
		return( INFINITYL );
	else
		return( 0.0L );
	}

if( nn == 0 )
	return( 1.0L );


if( x < 0.0L )
	{
	asign = -1;
	x = -x;
	}
else
	asign = 0;


if( nn < 0 )
	{
	sign = -1;
	n = -nn;
	}
else
	{
	sign = 1;
	n = nn;
	}

/* Overflow detection */

/* Calculate approximate logarithm of answer */
s = x;
s = frexpl( s, &lx );
e = (lx - 1)*n;
if( (e == 0) || (e > 64) || (e < -64) )
	{
	s = (s - 7.0710678118654752e-1L) / (s +  7.0710678118654752e-1L);
	s = (2.9142135623730950L * s - 0.5L + lx) * nn * LOGE2L;
	}
else
	{
	s = LOGE2L * e;
	}

if( s > MAXLOGL )
	{
	mtherr( "__powil", OVERFLOW );
	_SET_ERRNO(ERANGE);
	y = INFINITYL;
	goto done;
	}

if( s < MINLOGL )
	{
	mtherr( "__powil", UNDERFLOW );
	_SET_ERRNO(ERANGE);
	return(0.0L);
	}
/* Handle tiny denormal answer, but with less accuracy
 * since roundoff error in 1.0/x will be amplified.
 * The precise demarcation should be the gradual underflow threshold.
 */
if( s < (-MAXLOGL+2.0L) )
	{
	x = 1.0L/x;
	sign = -sign;
	}

/* First bit of the power */
if( n & 1 )
	y = x;
		
else
	{
	y = 1.0L;
	asign = 0;
	}

w = x;
n >>= 1;
while( n )
	{
	w = w * w;	/* arg to the 2-to-the-kth power */
	if( n & 1 )	/* if that bit is set, then include in product */
		y *= w;
	n >>= 1;
	}


done:

if( asign )
	y = -y; /* odd power of negative number */
if( sign < 0 )
	y = 1.0L/y;
return(y);
}
