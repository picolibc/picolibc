/*							powif.c
 *
 *	Real raised to integer power
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, powif();
 * int n;
 *
 * y = powif( x, n );
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
 *    IEEE      .04,26     -26,26    100000       1.1e-6      2.0e-7
 *    IEEE        1,2      -128,128  100000       1.1e-5      1.0e-6
 *
 * Returns MAXNUMF on overflow, zero on underflow.
 *
 */

/*							powi.c	*/

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1989 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"
extern float MAXNUMF, MAXLOGF, MINLOGF, LOGE2F;

#ifdef ANSIC
float frexpf( float, int * );

float powif( float x, int nn )
#else
float frexpf();

float powif( x, nn )
double x;
int nn;
#endif
{
int n, e, sign, asign, lx;
float w, y, s;

if( x == 0.0 )
	{
	if( nn == 0 )
		return( 1.0 );
	else if( nn < 0 )
		return( MAXNUMF );
	else
		return( 0.0 );
	}

if( nn == 0 )
	return( 1.0 );


if( x < 0.0 )
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
/*
	x = 1.0/x;
*/
	}
else
	{
	sign = 0;
	n = nn;
	}

/* Overflow detection */

/* Calculate approximate logarithm of answer */
s = frexpf( x, &lx );
e = (lx - 1)*n;
if( (e == 0) || (e > 64) || (e < -64) )
	{
	s = (s - 7.0710678118654752e-1) / (s +  7.0710678118654752e-1);
	s = (2.9142135623730950 * s - 0.5 + lx) * nn * LOGE2F;
	}
else
	{
	s = LOGE2F * e;
	}

if( s > MAXLOGF )
	{
	mtherr( "powi", OVERFLOW );
	y = MAXNUMF;
	goto done;
	}

if( s < MINLOGF )
	return(0.0);

/* Handle tiny denormal answer, but with less accuracy
 * since roundoff error in 1.0/x will be amplified.
 * The precise demarcation should be the gradual underflow threshold.
 */
if( s < (-MAXLOGF+2.0) )
	{
	x = 1.0/x;
	sign = 0;
	}

/* First bit of the power */
if( n & 1 )
	y = x;
		
else
	{
	y = 1.0;
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
if( sign )
	y = 1.0/y;
return(y);
}
