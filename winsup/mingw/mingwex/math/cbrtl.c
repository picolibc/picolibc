/*							cbrtl.c
 *
 *	Cube root, long double precision
 *
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, cbrtl();
 *
 * y = cbrtl( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the cube root of the argument, which may be negative.
 *
 * Range reduction involves determining the power of 2 of
 * the argument.  A polynomial of degree 2 applied to the
 * mantissa, and multiplication by the cube root of 1, 2, or 4
 * approximates the root to within about 0.1%.  Then Newton's
 * iteration is used three times to converge to an accurate
 * result.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE     .125,8        80000      7.0e-20     2.2e-20
 *    IEEE    exp(+-707)    100000      7.0e-20     2.4e-20
 *
 */


/*
Cephes Math Library Release 2.2: January, 1991
Copyright 1984, 1991 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/*
  Modified for mingwex.a 
  2002-07-01  Danny Smith  <dannysmith@users.sourceforge.net>
 */
#ifdef __MINGW32__
#include "cephes_mconf.h"
#else
#include "mconf.h"
#endif

static const long double CBRT2  = 1.2599210498948731647672L;
static const long double CBRT4  = 1.5874010519681994747517L;
static const long double CBRT2I = 0.79370052598409973737585L;
static const long double CBRT4I = 0.62996052494743658238361L;

#ifndef __MINGW32__

#ifdef ANSIPROT
extern long double frexpl ( long double, int * );
extern long double ldexpl ( long double, int );
extern int isnanl ( long double );
#else
long double frexpl(), ldexpl();
extern int isnanl();
#endif

#ifdef INFINITIES
extern long double INFINITYL;
#endif

#endif /* __MINGW32__ */

long double cbrtl(x)
long double x;
{
int e, rem, sign;
long double z;

#ifdef __MINGW32__
if (!isfinite (x) || x == 0.0L)
	return(x);
#else     

#ifdef NANS
if(isnanl(x))
	return(x);
#endif
#ifdef INFINITIES
if( x == INFINITYL)
	return(x);
if( x == -INFINITYL)
	return(x);
#endif
if( x == 0 )
	return( x );

#endif /* __MINGW32__ */

if( x > 0 )
	sign = 1;
else
	{
	sign = -1;
	x = -x;
	}

z = x;
/* extract power of 2, leaving
 * mantissa between 0.5 and 1
 */
x = frexpl( x, &e );

/* Approximate cube root of number between .5 and 1,
 * peak relative error = 1.2e-6
 */
x = (((( 1.3584464340920900529734e-1L * x
       - 6.3986917220457538402318e-1L) * x
       + 1.2875551670318751538055e0L) * x
       - 1.4897083391357284957891e0L) * x
       + 1.3304961236013647092521e0L) * x
       + 3.7568280825958912391243e-1L;

/* exponent divided by 3 */
if( e >= 0 )
	{
	rem = e;
	e /= 3;
	rem -= 3*e;
	if( rem == 1 )
		x *= CBRT2;
	else if( rem == 2 )
		x *= CBRT4;
	}
else
	{ /* argument less than 1 */
	e = -e;
	rem = e;
	e /= 3;
	rem -= 3*e;
	if( rem == 1 )
		x *= CBRT2I;
	else if( rem == 2 )
		x *= CBRT4I;
	e = -e;
	}

/* multiply by power of 2 */
x = ldexpl( x, e );

/* Newton iteration */

x -= ( x - (z/(x*x)) )*0.3333333333333333333333L;
x -= ( x - (z/(x*x)) )*0.3333333333333333333333L;

if( sign < 0 )
	x = -x;
return(x);
}
