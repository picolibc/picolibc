/*							powl.c
 *
 *	Power function, long double precision
 *
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, z, powl();
 *
 * z = powl( x, y );
 *
 *
 *
 * DESCRIPTION:
 *
 * Computes x raised to the yth power.  Analytically,
 *
 *      x**y  =  exp( y log(x) ).
 *
 * Following Cody and Waite, this program uses a lookup table
 * of 2**-i/32 and pseudo extended precision arithmetic to
 * obtain several extra bits of accuracy in both the logarithm
 * and the exponential.
 *
 *
 *
 * ACCURACY:
 *
 * The relative error of pow(x,y) can be estimated
 * by   y dl ln(2),   where dl is the absolute error of
 * the internally computed base 2 logarithm.  At the ends
 * of the approximation interval the logarithm equal 1/32
 * and its relative error is about 1 lsb = 1.1e-19.  Hence
 * the predicted relative error in the result is 2.3e-21 y .
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *
 *    IEEE     +-1000       40000      2.8e-18      3.7e-19
 * .001 < x < 1000, with log(x) uniformly distributed.
 * -1000 < y < 1000, y uniformly distributed.
 *
 *    IEEE     0,8700       60000      6.5e-18      1.0e-18
 * 0.99 < x < 1.01, 0 < y < 8700, uniformly distributed.
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * pow overflow     x**y > MAXNUM      INFINITY
 * pow underflow   x**y < 1/MAXNUM       0.0
 * pow domain      x<0 and y noninteger  0.0
 *
 */

/*
Cephes Math Library Release 2.7:  May, 1998
Copyright 1984, 1991, 1998 by Stephen L. Moshier
*/

/*
Modified for mingw
2002-07-22 Danny Smith <dannysmith@users.sourceforge.net>
*/

#ifdef __MINGW32__
#include "cephes_mconf.h"
#else
#include "mconf.h"

static char fname[] = {"powl"};
#endif

#ifndef _SET_ERRNO
#define _SET_ERRNO(x)
#endif


/* Table size */
#define NXT 32
/* log2(Table size) */
#define LNXT 5

#ifdef UNK
/* log(1+x) =  x - .5x^2 + x^3 *  P(z)/Q(z)
 * on the domain  2^(-1/32) - 1  <=  x  <=  2^(1/32) - 1
 */
static long double P[] = {
 8.3319510773868690346226E-4L,
 4.9000050881978028599627E-1L,
 1.7500123722550302671919E0L,
 1.4000100839971580279335E0L,
};
static long double Q[] = {
/* 1.0000000000000000000000E0L,*/
 5.2500282295834889175431E0L,
 8.4000598057587009834666E0L,
 4.2000302519914740834728E0L,
};
/* A[i] = 2^(-i/32), rounded to IEEE long double precision.
 * If i is even, A[i] + B[i/2] gives additional accuracy.
 */
static long double A[33] = {
 1.0000000000000000000000E0L,
 9.7857206208770013448287E-1L,
 9.5760328069857364691013E-1L,
 9.3708381705514995065011E-1L,
 9.1700404320467123175367E-1L,
 8.9735453750155359320742E-1L,
 8.7812608018664974155474E-1L,
 8.5930964906123895780165E-1L,
 8.4089641525371454301892E-1L,
 8.2287773907698242225554E-1L,
 8.0524516597462715409607E-1L,
 7.8799042255394324325455E-1L,
 7.7110541270397041179298E-1L,
 7.5458221379671136985669E-1L,
 7.3841307296974965571198E-1L,
 7.2259040348852331001267E-1L,
 7.0710678118654752438189E-1L,
 6.9195494098191597746178E-1L,
 6.7712777346844636413344E-1L,
 6.6261832157987064729696E-1L,
 6.4841977732550483296079E-1L,
 6.3452547859586661129850E-1L,
 6.2092890603674202431705E-1L,
 6.0762367999023443907803E-1L,
 5.9460355750136053334378E-1L,
 5.8186242938878875689693E-1L,
 5.6939431737834582684856E-1L,
 5.5719337129794626814472E-1L,
 5.4525386633262882960438E-1L,
 5.3357020033841180906486E-1L,
 5.2213689121370692017331E-1L,
 5.1094857432705833910408E-1L,
 5.0000000000000000000000E-1L,
};
static long double B[17] = {
 0.0000000000000000000000E0L,
 2.6176170809902549338711E-20L,
-1.0126791927256478897086E-20L,
 1.3438228172316276937655E-21L,
 1.2207982955417546912101E-20L,
-6.3084814358060867200133E-21L,
 1.3164426894366316434230E-20L,
-1.8527916071632873716786E-20L,
 1.8950325588932570796551E-20L,
 1.5564775779538780478155E-20L,
 6.0859793637556860974380E-21L,
-2.0208749253662532228949E-20L,
 1.4966292219224761844552E-20L,
 3.3540909728056476875639E-21L,
-8.6987564101742849540743E-22L,
-1.2327176863327626135542E-20L,
 0.0000000000000000000000E0L,
};

/* 2^x = 1 + x P(x),
 * on the interval -1/32 <= x <= 0
 */
static long double R[] = {
 1.5089970579127659901157E-5L,
 1.5402715328927013076125E-4L,
 1.3333556028915671091390E-3L,
 9.6181291046036762031786E-3L,
 5.5504108664798463044015E-2L,
 2.4022650695910062854352E-1L,
 6.9314718055994530931447E-1L,
};

#define douba(k) A[k]
#define doubb(k) B[k]
#define MEXP (NXT*16384.0L)
/* The following if denormal numbers are supported, else -MEXP: */
#ifdef DENORMAL
#define MNEXP (-NXT*(16384.0L+64.0L))
#else
#define MNEXP (-NXT*16384.0L)
#endif
/* log2(e) - 1 */
#define LOG2EA 0.44269504088896340735992L
#endif


#ifdef IBMPC
static const uLD P[] = {
{ { 0xb804,0xa8b7,0xc6f4,0xda6a,0x3ff4, XPD } },
{ { 0x7de9,0xcf02,0x58c0,0xfae1,0x3ffd, XPD } },
{ { 0x405a,0x3722,0x67c9,0xe000,0x3fff, XPD } },
{ { 0xcd99,0x6b43,0x87ca,0xb333,0x3fff, XPD } }
};
static const uLD Q[] = {
{ { 0x6307,0xa469,0x3b33,0xa800,0x4001, XPD } },
{ { 0xfec2,0x62d7,0xa51c,0x8666,0x4002, XPD } },
{ { 0xda32,0xd072,0xa5d7,0x8666,0x4001, XPD } }
};
static const uLD A[] = {
{ { 0x0000,0x0000,0x0000,0x8000,0x3fff, XPD } },
{ { 0x033a,0x722a,0xb2db,0xfa83,0x3ffe, XPD } },
{ { 0xcc2c,0x2486,0x7d15,0xf525,0x3ffe, XPD } },
{ { 0xf5cb,0xdcda,0xb99b,0xefe4,0x3ffe, XPD } },
{ { 0x392f,0xdd24,0xc6e7,0xeac0,0x3ffe, XPD } },
{ { 0x48a8,0x7c83,0x06e7,0xe5b9,0x3ffe, XPD } },
{ { 0xe111,0x2a94,0xdeec,0xe0cc,0x3ffe, XPD } },
{ { 0x3755,0xdaf2,0xb797,0xdbfb,0x3ffe, XPD } },
{ { 0x6af4,0xd69d,0xfcca,0xd744,0x3ffe, XPD } },
{ { 0xe45a,0xf12a,0x1d91,0xd2a8,0x3ffe, XPD } },
{ { 0x80e4,0x1f84,0x8c15,0xce24,0x3ffe, XPD } },
{ { 0x27a3,0x6e2f,0xbd86,0xc9b9,0x3ffe, XPD } },
{ { 0xdadd,0x5506,0x2a11,0xc567,0x3ffe, XPD } },
{ { 0x9456,0x6670,0x4cca,0xc12c,0x3ffe, XPD } },
{ { 0x36bf,0x580c,0xa39f,0xbd08,0x3ffe, XPD } },
{ { 0x9ee9,0x62fb,0xaf47,0xb8fb,0x3ffe, XPD } },
{ { 0x6484,0xf9de,0xf333,0xb504,0x3ffe, XPD } },
{ { 0x2590,0xd2ac,0xf581,0xb123,0x3ffe, XPD } },
{ { 0x4ac6,0x42a1,0x3eea,0xad58,0x3ffe, XPD } },
{ { 0x0ef8,0xea7c,0x5ab4,0xa9a1,0x3ffe, XPD } },
{ { 0x38ea,0xb151,0xd6a9,0xa5fe,0x3ffe, XPD } },
{ { 0x6819,0x0c49,0x4303,0xa270,0x3ffe, XPD } },
{ { 0x11ae,0x91a1,0x3260,0x9ef5,0x3ffe, XPD } },
{ { 0x5539,0xd54e,0x39b9,0x9b8d,0x3ffe, XPD } },
{ { 0xa96f,0x8db8,0xf051,0x9837,0x3ffe, XPD } },
{ { 0x0961,0xfef7,0xefa8,0x94f4,0x3ffe, XPD } },
{ { 0xc336,0xab11,0xd373,0x91c3,0x3ffe, XPD } },
{ { 0x53c0,0x45cd,0x398b,0x8ea4,0x3ffe, XPD } },
{ { 0xd6e7,0xea8b,0xc1e3,0x8b95,0x3ffe, XPD } },
{ { 0x8527,0x92da,0x0e80,0x8898,0x3ffe, XPD } },
{ { 0x7b15,0xcc48,0xc367,0x85aa,0x3ffe, XPD } },
{ { 0xa1d7,0xac2b,0x8698,0x82cd,0x3ffe, XPD } },
{ { 0x0000,0x0000,0x0000,0x8000,0x3ffe, XPD } }
};
static const uLD B[] = {
{ { 0x0000,0x0000,0x0000,0x0000,0x0000, XPD } },
{ { 0x1f87,0xdb30,0x18f5,0xf73a,0x3fbd, XPD } },
{ { 0xac15,0x3e46,0x2932,0xbf4a,0xbfbc, XPD } },
{ { 0x7944,0xba66,0xa091,0xcb12,0x3fb9, XPD } },
{ { 0xff78,0x40b4,0x2ee6,0xe69a,0x3fbc, XPD } },
{ { 0xc895,0x5069,0xe383,0xee53,0xbfbb, XPD } },
{ { 0x7cde,0x9376,0x4325,0xf8ab,0x3fbc, XPD } },
{ { 0xa10c,0x25e0,0xc093,0xaefd,0xbfbd, XPD } },
{ { 0x7d3e,0xea95,0x1366,0xb2fb,0x3fbd, XPD } },
{ { 0x5d89,0xeb34,0x5191,0x9301,0x3fbd, XPD } },
{ { 0x80d9,0xb883,0xfb10,0xe5eb,0x3fbb, XPD } },
{ { 0x045d,0x288c,0xc1ec,0xbedd,0xbfbd, XPD } },
{ { 0xeded,0x5c85,0x4630,0x8d5a,0x3fbd, XPD } },
{ { 0x9d82,0xe5ac,0x8e0a,0xfd6d,0x3fba, XPD } },
{ { 0x6dfd,0xeb58,0xaf14,0x8373,0xbfb9, XPD } },
{ { 0xf938,0x7aac,0x91cf,0xe8da,0xbfbc, XPD } },
{ { 0x0000,0x0000,0x0000,0x0000,0x0000, XPD } }
};
static const uLD R[] = {
{ { 0xa69b,0x530e,0xee1d,0xfd2a,0x3fee, XPD } },
{ { 0xc746,0x8e7e,0x5960,0xa182,0x3ff2, XPD } },
{ { 0x63b6,0xadda,0xfd6a,0xaec3,0x3ff5, XPD } },
{ { 0xc104,0xfd99,0x5b7c,0x9d95,0x3ff8, XPD } },
{ { 0xe05e,0x249d,0x46b8,0xe358,0x3ffa, XPD } },
{ { 0x5d1d,0x162c,0xeffc,0xf5fd,0x3ffc, XPD } },
{ { 0x79aa,0xd1cf,0x17f7,0xb172,0x3ffe, XPD } }
};

/* 10 byte sizes versus 12 byte */
#define douba(k) (A[(k)].ld)
#define doubb(k) (B[(k)].ld)
#define MEXP (NXT*16384.0L)
#ifdef DENORMAL
#define MNEXP (-NXT*(16384.0L+64.0L))
#else
#define MNEXP (-NXT*16384.0L)
#endif
static const
union
{
  unsigned short L[6];
  long double ld;
} log2ea = {{0xc2ef,0x705f,0xeca5,0xe2a8,0x3ffd, XPD}};

#define LOG2EA (log2ea.ld)
/*
#define LOG2EA 0.44269504088896340735992L
*/
#endif

#ifdef MIEEE
static long P[] = {
0x3ff40000,0xda6ac6f4,0xa8b7b804,
0x3ffd0000,0xfae158c0,0xcf027de9,
0x3fff0000,0xe00067c9,0x3722405a,
0x3fff0000,0xb33387ca,0x6b43cd99,
};
static long Q[] = {
/* 0x3fff0000,0x80000000,0x00000000, */
0x40010000,0xa8003b33,0xa4696307,
0x40020000,0x8666a51c,0x62d7fec2,
0x40010000,0x8666a5d7,0xd072da32,
};
static long A[] = {
0x3fff0000,0x80000000,0x00000000,
0x3ffe0000,0xfa83b2db,0x722a033a,
0x3ffe0000,0xf5257d15,0x2486cc2c,
0x3ffe0000,0xefe4b99b,0xdcdaf5cb,
0x3ffe0000,0xeac0c6e7,0xdd24392f,
0x3ffe0000,0xe5b906e7,0x7c8348a8,
0x3ffe0000,0xe0ccdeec,0x2a94e111,
0x3ffe0000,0xdbfbb797,0xdaf23755,
0x3ffe0000,0xd744fcca,0xd69d6af4,
0x3ffe0000,0xd2a81d91,0xf12ae45a,
0x3ffe0000,0xce248c15,0x1f8480e4,
0x3ffe0000,0xc9b9bd86,0x6e2f27a3,
0x3ffe0000,0xc5672a11,0x5506dadd,
0x3ffe0000,0xc12c4cca,0x66709456,
0x3ffe0000,0xbd08a39f,0x580c36bf,
0x3ffe0000,0xb8fbaf47,0x62fb9ee9,
0x3ffe0000,0xb504f333,0xf9de6484,
0x3ffe0000,0xb123f581,0xd2ac2590,
0x3ffe0000,0xad583eea,0x42a14ac6,
0x3ffe0000,0xa9a15ab4,0xea7c0ef8,
0x3ffe0000,0xa5fed6a9,0xb15138ea,
0x3ffe0000,0xa2704303,0x0c496819,
0x3ffe0000,0x9ef53260,0x91a111ae,
0x3ffe0000,0x9b8d39b9,0xd54e5539,
0x3ffe0000,0x9837f051,0x8db8a96f,
0x3ffe0000,0x94f4efa8,0xfef70961,
0x3ffe0000,0x91c3d373,0xab11c336,
0x3ffe0000,0x8ea4398b,0x45cd53c0,
0x3ffe0000,0x8b95c1e3,0xea8bd6e7,
0x3ffe0000,0x88980e80,0x92da8527,
0x3ffe0000,0x85aac367,0xcc487b15,
0x3ffe0000,0x82cd8698,0xac2ba1d7,
0x3ffe0000,0x80000000,0x00000000,
};
static long B[51] = {
0x00000000,0x00000000,0x00000000,
0x3fbd0000,0xf73a18f5,0xdb301f87,
0xbfbc0000,0xbf4a2932,0x3e46ac15,
0x3fb90000,0xcb12a091,0xba667944,
0x3fbc0000,0xe69a2ee6,0x40b4ff78,
0xbfbb0000,0xee53e383,0x5069c895,
0x3fbc0000,0xf8ab4325,0x93767cde,
0xbfbd0000,0xaefdc093,0x25e0a10c,
0x3fbd0000,0xb2fb1366,0xea957d3e,
0x3fbd0000,0x93015191,0xeb345d89,
0x3fbb0000,0xe5ebfb10,0xb88380d9,
0xbfbd0000,0xbeddc1ec,0x288c045d,
0x3fbd0000,0x8d5a4630,0x5c85eded,
0x3fba0000,0xfd6d8e0a,0xe5ac9d82,
0xbfb90000,0x8373af14,0xeb586dfd,
0xbfbc0000,0xe8da91cf,0x7aacf938,
0x00000000,0x00000000,0x00000000,
};
static long R[] = {
0x3fee0000,0xfd2aee1d,0x530ea69b,
0x3ff20000,0xa1825960,0x8e7ec746,
0x3ff50000,0xaec3fd6a,0xadda63b6,
0x3ff80000,0x9d955b7c,0xfd99c104,
0x3ffa0000,0xe35846b8,0x249de05e,
0x3ffc0000,0xf5fdeffc,0x162c5d1d,
0x3ffe0000,0xb17217f7,0xd1cf79aa,
};

#define douba(k) (*(long double *)&A[3*(k)])
#define doubb(k) (*(long double *)&B[3*(k)])
#define MEXP (NXT*16384.0L)
#ifdef DENORMAL
#define MNEXP (-NXT*(16384.0L+64.0L))
#else
#define MNEXP (-NXT*16382.0L)
#endif
static long L[3] = {0x3ffd0000,0xe2a8eca5,0x705fc2ef};
#define LOG2EA (*(long double *)(&L[0]))
#endif


#define F W
#define Fa Wa
#define Fb Wb
#define G W
#define Ga Wa
#define Gb u
#define H W
#define Ha Wb
#define Hb Wb

#ifndef __MINGW32__
extern long double MAXNUML;
#endif

static VOLATILE long double z;
static long double w, W, Wa, Wb, ya, yb, u;

#ifdef __MINGW32__
static __inline__ long double reducl( long double );
extern long double __powil ( long double, int );
extern long double powl ( long double x, long double y);
#else
#ifdef ANSIPROT
extern long double floorl ( long double );
extern long double fabsl ( long double );
extern long double frexpl ( long double, int * );
extern long double ldexpl ( long double, int );
extern long double polevll ( long double, void *, int );
extern long double p1evll ( long double, void *, int );
extern long double __powil ( long double, int );
extern int isnanl ( long double );
extern int isfinitel ( long double );
static long double reducl( long double );
extern int signbitl ( long double );
#else
long double floorl(), fabsl(), frexpl(), ldexpl();
long double polevll(), p1evll(), __powil();
static long double reducl();
int isnanl(), isfinitel(), signbitl();
#endif  /* __MINGW32__ */

#ifdef INFINITIES
extern long double INFINITYL;
#else
#define INFINITYL MAXNUML
#endif

#ifdef NANS
extern long double NANL;
#endif
#ifdef MINUSZERO
extern long double NEGZEROL;
#endif

#endif /* __MINGW32__ */

#ifdef __MINGW32__

/* No error checking. We handle Infs and zeros ourselves.  */
static __inline__ long double
__fast_ldexpl (long double x, int expn)
{
  long double res;
  __asm__ ("fscale"
  	    : "=t" (res)
	    : "0" (x), "u" ((long double) expn));
  return res;
}

#define ldexpl  __fast_ldexpl

#endif


long double powl( x, y )
long double x, y;
{
/* double F, Fa, Fb, G, Ga, Gb, H, Ha, Hb */
int i, nflg, iyflg, yoddint;
long e;

if( y == 0.0L )
	return( 1.0L );

#ifdef NANS
if( isnanl(x) )
	{
	_SET_ERRNO (EDOM);
	return( x );
	}
if( isnanl(y) )
	{
	_SET_ERRNO (EDOM);
	return( y );
	}
#endif

if( y == 1.0L )
	return( x );

if( isinfl(y) && (x == -1.0L || x == 1.0L) )
	return( y );

if( x == 1.0L )
	return( 1.0L );

if( y >= MAXNUML )
	{
	_SET_ERRNO (ERANGE);
#ifdef INFINITIES
	if( x > 1.0L )
		return( INFINITYL );
#else
	if( x > 1.0L )
		return( MAXNUML );
#endif
	if( x > 0.0L && x < 1.0L )
		return( 0.0L );
#ifdef INFINITIES
	if( x < -1.0L )
		return( INFINITYL );
#else
	if( x < -1.0L )
		return( MAXNUML );
#endif
	if( x > -1.0L && x < 0.0L )
		return( 0.0L );
	}
if( y <= -MAXNUML )
	{
	_SET_ERRNO (ERANGE);
	if( x > 1.0L )
		return( 0.0L );
#ifdef INFINITIES
	if( x > 0.0L && x < 1.0L )
		return( INFINITYL );
#else
	if( x > 0.0L && x < 1.0L )
		return( MAXNUML );
#endif
	if( x < -1.0L )
		return( 0.0L );
#ifdef INFINITIES
	if( x > -1.0L && x < 0.0L )
		return( INFINITYL );
#else
	if( x > -1.0L && x < 0.0L )
		return( MAXNUML );
#endif
	}
if( x >= MAXNUML )
	{
#if INFINITIES
	if( y > 0.0L )
		return( INFINITYL );
#else
	if( y > 0.0L )
		return( MAXNUML );
#endif
	return( 0.0L );
	}

w = floorl(y);
/* Set iyflg to 1 if y is an integer.  */
iyflg = 0;
if( w == y )
	iyflg = 1;

/* Test for odd integer y.  */
yoddint = 0;
if( iyflg )
	{
	ya = fabsl(y);
	ya = floorl(0.5L * ya);
	yb = 0.5L * fabsl(w);
	if( ya != yb )
		yoddint = 1;
	}

if( x <= -MAXNUML )
	{
	if( y > 0.0L )
		{
#ifdef INFINITIES
		if( yoddint )
			return( -INFINITYL );
		return( INFINITYL );
#else
		if( yoddint )
			return( -MAXNUML );
		return( MAXNUML );
#endif
		}
	if( y < 0.0L )
		{
#ifdef MINUSZERO
		if( yoddint )
			return( NEGZEROL );
#endif
		return( 0.0 );
		}
 	}


nflg = 0;	/* flag = 1 if x<0 raised to integer power */
if( x <= 0.0L )
	{
	if( x == 0.0L )
		{
		if( y < 0.0 )
			{
#ifdef MINUSZERO
			if( signbitl(x) && yoddint )
				return( -INFINITYL );
#endif
#ifdef INFINITIES
			return( INFINITYL );
#else
			return( MAXNUML );
#endif
			}
		if( y > 0.0 )
			{
#ifdef MINUSZERO
			if( signbitl(x) && yoddint )
				return( NEGZEROL );
#endif
			return( 0.0 );
			}
		if( y == 0.0L )
			return( 1.0L );  /*   0**0   */
		else  
			return( 0.0L );  /*   0**y   */
		}
	else
		{
		if( iyflg == 0 )
			{ /* noninteger power of negative number */
			mtherr( fname, DOMAIN );
			_SET_ERRNO (EDOM);
#ifdef NANS
			return(NANL);
#else
			return(0.0L);
#endif
			}
		nflg = 1;
		}
	}

/* Integer power of an integer.  */

if( iyflg )
	{
	i = w;
	w = floorl(x);
	if( (w == x) && (fabsl(y) < 32768.0) )
		{
		w = __powil( x, (int) y );
		return( w );
		}
	}


if( nflg )
	x = fabsl(x);

/* separate significand from exponent */
x = frexpl( x, &i );
e = i;

/* find significand in antilog table A[] */
i = 1;
if( x <= douba(17) )
	i = 17;
if( x <= douba(i+8) )
	i += 8;
if( x <= douba(i+4) )
	i += 4;
if( x <= douba(i+2) )
	i += 2;
if( x >= douba(1) )
	i = -1;
i += 1;


/* Find (x - A[i])/A[i]
 * in order to compute log(x/A[i]):
 *
 * log(x) = log( a x/a ) = log(a) + log(x/a)
 *
 * log(x/a) = log(1+v),  v = x/a - 1 = (x-a)/a
 */
x -= douba(i);
x -= doubb(i/2);
x /= douba(i);


/* rational approximation for log(1+v):
 *
 * log(1+v)  =  v  -  v**2/2  +  v**3 P(v) / Q(v)
 */
z = x*x;
w = x * ( z * polevll( x, P, 3 ) / p1evll( x, Q, 3 ) );
w = w - ldexpl( z, -1 );   /*  w - 0.5 * z  */

/* Convert to base 2 logarithm:
 * multiply by log2(e) = 1 + LOG2EA
 */
z = LOG2EA * w;
z += w;
z += LOG2EA * x;
z += x;

/* Compute exponent term of the base 2 logarithm. */
w = -i;
w = ldexpl( w, -LNXT );	/* divide by NXT */
w += e;
/* Now base 2 log of x is w + z. */

/* Multiply base 2 log by y, in extended precision. */

/* separate y into large part ya
 * and small part yb less than 1/NXT
 */
ya = reducl(y);
yb = y - ya;

/* (w+z)(ya+yb)
 * = w*ya + w*yb + z*y
 */
F = z * y  +  w * yb;
Fa = reducl(F);
Fb = F - Fa;

G = Fa + w * ya;
Ga = reducl(G);
Gb = G - Ga;

H = Fb + Gb;
Ha = reducl(H);
w = ldexpl( Ga + Ha, LNXT );

/* Test the power of 2 for overflow */
if( w > MEXP )
	{
	_SET_ERRNO (ERANGE);
	mtherr( fname, OVERFLOW );
	return( MAXNUML );
	}

if( w < MNEXP )
	{
	_SET_ERRNO (ERANGE);
	mtherr( fname, UNDERFLOW );
	return( 0.0L );
	}

e = w;
Hb = H - Ha;

if( Hb > 0.0L )
	{
	e += 1;
	Hb -= (1.0L/NXT);  /*0.0625L;*/
	}

/* Now the product y * log2(x)  =  Hb + e/NXT.
 *
 * Compute base 2 exponential of Hb,
 * where -0.0625 <= Hb <= 0.
 */
z = Hb * polevll( Hb, R, 6 );  /*    z  =  2**Hb - 1    */

/* Express e/NXT as an integer plus a negative number of (1/NXT)ths.
 * Find lookup table entry for the fractional power of 2.
 */
if( e < 0 )
	i = 0;
else
	i = 1;
i = e/NXT + i;
e = NXT*i - e;
w = douba( e );
z = w * z;      /*    2**-e * ( 1 + (2**Hb-1) )    */
z = z + w;
z = ldexpl( z, i );  /* multiply by integer power of 2 */

if( nflg )
	{
/* For negative x,
 * find out if the integer exponent
 * is odd or even.
 */
	w = ldexpl( y, -1 );
	w = floorl(w);
	w = ldexpl( w, 1 );
	if( w != y )
		z = -z; /* odd exponent */
	}

return( z );
}

static __inline__ long double 
__convert_inf_to_maxnum(long double x)
{
  if (isinf(x))
    return (x > 0.0L ? MAXNUML : -MAXNUML);
  else
    return x;
}


/* Find a multiple of 1/NXT that is within 1/NXT of x. */
static __inline__ long double reducl(x)
long double x;
{
long double t;

/* If the call to ldexpl overflows, set it to MAXNUML.
   This avoids Inf - Inf = Nan result when calculating the 'small'
   part of a reduction.  Instead, the small part becomes Inf,
   causing under/overflow when adding it to the 'large' part.
   There must be a cleaner way of doing this. */
t =  __convert_inf_to_maxnum (ldexpl( x, LNXT ));
t = floorl( t );
t = ldexpl( t, -LNXT );
return(t);
}
