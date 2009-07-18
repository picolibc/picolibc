/*							gamma.c
 *
 *	Gamma function
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, __tgamma_r();
 * int* sgngam;
 * y = __tgamma_r( x, sgngam );
 * 
 * double x, y, tgamma();
 * y = tgamma( x)
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns gamma function of the argument.  The result is
 * correctly signed. In the reentrant version the sign (+1 or -1)
 * is returned in the variable referenced by sgngam.
 *
 * Arguments |x| <= 34 are reduced by recurrence and the function
 * approximated by a rational function of degree 6/7 in the
 * interval (2,3).  Large arguments are handled by Stirling's
 * formula. Large negative arguments are made positive using
 * a reflection formula.  
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    DEC      -34, 34      10000       1.3e-16     2.5e-17
 *    IEEE    -170,-33      20000       2.3e-15     3.3e-16
 *    IEEE     -33,  33     20000       9.4e-16     2.2e-16
 *    IEEE      33, 171.6   20000       2.3e-15     3.2e-16
 *
 * Error for arguments outside the test range will be larger
 * owing to error amplification by the exponential function.
 *
 */

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1989, 1992, 2000 by Stephen L. Moshier
*/


/*
 * 26-11-2002 Modified for mingw.
 * Danny Smith <dannysmith@users.sourceforge.net>
 */


#ifndef __MINGW32__
#include "mconf.h"
#else
#include "cephes_mconf.h"
#endif

#ifdef UNK
static const double P[] = {
  1.60119522476751861407E-4,
  1.19135147006586384913E-3,
  1.04213797561761569935E-2,
  4.76367800457137231464E-2,
  2.07448227648435975150E-1,
  4.94214826801497100753E-1,
  9.99999999999999996796E-1
};
static const double Q[] = {
-2.31581873324120129819E-5,
 5.39605580493303397842E-4,
-4.45641913851797240494E-3,
 1.18139785222060435552E-2,
 3.58236398605498653373E-2,
-2.34591795718243348568E-1,
 7.14304917030273074085E-2,
 1.00000000000000000320E0
};
#define MAXGAM 171.624376956302725
static const double LOGPI = 1.14472988584940017414;
#endif

#ifdef DEC
static const unsigned short P[] = {
0035047,0162701,0146301,0005234,
0035634,0023437,0032065,0176530,
0036452,0137157,0047330,0122574,
0037103,0017310,0143041,0017232,
0037524,0066516,0162563,0164605,
0037775,0004671,0146237,0014222,
0040200,0000000,0000000,0000000
};
static const unsigned short Q[] = {
0134302,0041724,0020006,0116565,
0035415,0072121,0044251,0025634,
0136222,0003447,0035205,0121114,
0036501,0107552,0154335,0104271,
0037022,0135717,0014776,0171471,
0137560,0034324,0165024,0037021,
0037222,0045046,0047151,0161213,
0040200,0000000,0000000,0000000
};
#define MAXGAM 34.84425627277176174
#endif

#ifdef IBMPC
static const uD P[] = {
{ { 0x2153,0x3998,0xfcb8,0x3f24 } },
{ { 0xbfab,0xe686,0x84e3,0x3f53 } },
{ { 0x14b0,0xe9db,0x57cd,0x3f85 } },
{ { 0x23d3,0x18c4,0x63d9,0x3fa8 } },
{ { 0x7d31,0xdcae,0x8da9,0x3fca } },
{ { 0xe312,0x3993,0xa137,0x3fdf } },
{ { 0x0000,0x0000,0x0000,0x3ff0 } }
};
static const uD Q[] = {
{ { 0xd3af,0x8400,0x487a,0xbef8 } },
{ { 0x2573,0x2915,0xae8a,0x3f41 } },
{ { 0xb44a,0xe750,0x40e4,0xbf72 } },
{ { 0xb117,0x5b1b,0x31ed,0x3f88 } },
{ { 0xde67,0xe33f,0x5779,0x3fa2 } },
{ { 0x87c2,0x9d42,0x071a,0xbfce } },
{ { 0x3c51,0xc9cd,0x4944,0x3fb2 } },
{ { 0x0000,0x0000,0x0000,0x3ff0 } }
};
#define MAXGAM 171.624376956302725
#endif 

#ifdef MIEEE
static const unsigned short P[] = {
0x3f24,0xfcb8,0x3998,0x2153,
0x3f53,0x84e3,0xe686,0xbfab,
0x3f85,0x57cd,0xe9db,0x14b0,
0x3fa8,0x63d9,0x18c4,0x23d3,
0x3fca,0x8da9,0xdcae,0x7d31,
0x3fdf,0xa137,0x3993,0xe312,
0x3ff0,0x0000,0x0000,0x0000
};
static const unsigned short Q[] = {
0xbef8,0x487a,0x8400,0xd3af,
0x3f41,0xae8a,0x2915,0x2573,
0xbf72,0x40e4,0xe750,0xb44a,
0x3f88,0x31ed,0x5b1b,0xb117,
0x3fa2,0x5779,0xe33f,0xde67,
0xbfce,0x071a,0x9d42,0x87c2,
0x3fb2,0x4944,0xc9cd,0x3c51,
0x3ff0,0x0000,0x0000,0x0000
};
#define MAXGAM 171.624376956302725
#endif 

/* Stirling's formula for the gamma function */
#if UNK
static const double STIR[5] = {
 7.87311395793093628397E-4,
-2.29549961613378126380E-4,
-2.68132617805781232825E-3,
 3.47222221605458667310E-3,
 8.33333333333482257126E-2,
};
#define MAXSTIR 143.01608
static const double SQTPI = 2.50662827463100050242E0;
#endif
#if DEC
static const unsigned short STIR[20] = {
0035516,0061622,0144553,0112224,
0135160,0131531,0037460,0165740,
0136057,0134460,0037242,0077270,
0036143,0107070,0156306,0027751,
0037252,0125252,0125252,0146064,
};
#define MAXSTIR 26.77
static const unsigned short SQT[4] = {
0040440,0066230,0177661,0034055,
};
#define SQTPI *(double *)SQT
#endif
#if IBMPC
static const uD STIR[] = {
{ { 0x7293,0x592d,0xcc72,0x3f49 } },
{ { 0x1d7c,0x27e6,0x166b,0xbf2e } },
{ { 0x4fd7,0x07d4,0xf726,0xbf65 } },
{ { 0xc5fd,0x1b98,0x71c7,0x3f6c } },
{ { 0x5986,0x5555,0x5555,0x3fb5 } }
};
#define MAXSTIR 143.01608

static const union
{
  unsigned short s[4];
  double d;
} sqt = {{0x2706,0x1ff6,0x0d93,0x4004}};
#define SQTPI (sqt.d)
#endif
#if MIEEE
static const unsigned short STIR[20] = {
0x3f49,0xcc72,0x592d,0x7293,
0xbf2e,0x166b,0x27e6,0x1d7c,
0xbf65,0xf726,0x07d4,0x4fd7,
0x3f6c,0x71c7,0x1b98,0xc5fd,
0x3fb5,0x5555,0x5555,0x5986,
};
#define MAXSTIR 143.01608
static const unsigned short SQT[4] = {
0x4004,0x0d93,0x1ff6,0x2706,
};
#define SQTPI *(double *)SQT
#endif

#ifndef __MINGW32__
int sgngam = 0;
extern int sgngam;
extern double MAXLOG, MAXNUM, PI;
#ifdef ANSIPROT
extern double pow ( double, double );
extern double log ( double );
extern double exp ( double );
extern double sin ( double );
extern double polevl ( double, void *, int );
extern double p1evl ( double, void *, int );
extern double floor ( double );
extern double fabs ( double );
extern int isnan ( double );
extern int isfinite ( double );
static double stirf ( double );
double lgam ( double );
#else
double pow(), log(), exp(), sin(), polevl(), p1evl(), floor(), fabs();
int isnan(), isfinite();
static double stirf();
double lgam();
#endif
#ifdef INFINITIES
extern double INFINITY;
#endif
#ifdef NANS
extern double NAN;
#endif
#else /* __MINGW32__ */
static double stirf ( double );
#endif

/* Gamma function computed by Stirling's formula.
 * The polynomial STIR is valid for 33 <= x <= 172.
 */
static double stirf(x)
double x;
{
double y, w, v;

w = 1.0/x;
w = 1.0 + w * polevl( w, STIR, 4 );
y = exp(x);
if( x > MAXSTIR )
	{ /* Avoid overflow in pow() */
	v = pow( x, 0.5 * x - 0.25 );
	y = v * (v / y);
	}
else
	{
	y = pow( x, x - 0.5 ) / y;
	}
y = SQTPI * y * w;
return( y );
}



double __tgamma_r(double x, int* sgngam)
{
double p, q, z;
int i;

*sgngam = 1;
#ifdef NANS
if( isnan(x) )
	return(x);
#endif
#ifdef INFINITIES
#ifdef NANS
if( x == INFINITY )
	return(x);
if( x == -INFINITY )
	return(NAN);
#else
if( !isfinite(x) )
	return(x);
#endif
#endif
q = fabs(x);

if( q > 33.0 )
	{
	if( x < 0.0 )
		{
		p = floor(q);
		if( p == q )
			{
gsing:
			_SET_ERRNO(EDOM);
			mtherr( "tgamma", SING );
#ifdef INFINITIES
			return (INFINITY);
#else
			return (MAXNUM);
#endif
			}
		i = p;
		if( (i & 1) == 0 )
			*sgngam = -1;
		z = q - p;
		if( z > 0.5 )
			{
			p += 1.0;
			z = q - p;
			}
		z = q * sin( PI * z );
		if( z == 0.0 )
			{
			_SET_ERRNO(ERANGE);
			mtherr( "tgamma", OVERFLOW );
#ifdef INFINITIES
			return( *sgngam * INFINITY);
#else
			return( *sgngam * MAXNUM);
#endif
			}
		z = fabs(z);
		z = PI/(z * stirf(q) );
		}
	else
		{
		z = stirf(x);
		}
	return( *sgngam * z );
	}

z = 1.0;
while( x >= 3.0 )
	{
	x -= 1.0;
	z *= x;
	}

while( x < 0.0 )
	{
	if( x > -1.E-9 )
		goto Small;
	z /= x;
	x += 1.0;
	}

while( x < 2.0 )
	{
	if( x < 1.e-9 )
		goto Small;
	z /= x;
	x += 1.0;
	}

if( x == 2.0 )
	return(z);

x -= 2.0;
p = polevl( x, P, 6 );
q = polevl( x, Q, 7 );
return( z * p / q );

Small:
if( x == 0.0 )
	{
	goto gsing;
	}
else
	return( z/((1.0 + 0.5772156649015329 * x) * x) );
}

/* This is the C99 version */

double tgamma(double x)
{
  int local_sgngam=0;
  return (__tgamma_r(x, &local_sgngam));
}
