/* This file is extracted from S L Moshier's  ioldoubl.c,
 * modified for use in MinGW 
 *
 * Extended precision arithmetic functions for long double I/O.
 * This program has been placed in the public domain.
 */
  

/*
 * Revision history:
 *
 *  5 Jan 84	PDP-11 assembly language version
 *  6 Dec 86	C language version
 * 30 Aug 88	100 digit version, improved rounding
 * 15 May 92    80-bit long double support
 *
 * Author:  S. L. Moshier.
 *
 * 6 Oct 02	Modified for MinGW by inlining utility routines,
 * 		removing global variables and splitting out strtold
 *		from _IO_ldtoa and _IO_ldtostr.
 *  
 *		Danny Smith <dannysmith@users.sourceforge.net>
 * 
 */
 

#ifdef USE_LDTOA

#include "math/cephes_emath.h"

#if NE == 10

/* 1.0E0 */
static const unsigned short __eone[NE] =
 {0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x3fff,};

#else

static const unsigned short __eone[NE] = {
0, 0000000,0000000,0000000,0100000,0x3fff,};
#endif


#if NE == 10
static const unsigned short __etens[NTEN + 1][NE] =
{
  {0x6576, 0x4a92, 0x804a, 0x153f,
   0xc94c, 0x979a, 0x8a20, 0x5202, 0xc460, 0x7525,},	/* 10**4096 */
  {0x6a32, 0xce52, 0x329a, 0x28ce,
   0xa74d, 0x5de4, 0xc53d, 0x3b5d, 0x9e8b, 0x5a92,},	/* 10**2048 */
  {0x526c, 0x50ce, 0xf18b, 0x3d28,
   0x650d, 0x0c17, 0x8175, 0x7586, 0xc976, 0x4d48,},
  {0x9c66, 0x58f8, 0xbc50, 0x5c54,
   0xcc65, 0x91c6, 0xa60e, 0xa0ae, 0xe319, 0x46a3,},
  {0x851e, 0xeab7, 0x98fe, 0x901b,
   0xddbb, 0xde8d, 0x9df9, 0xebfb, 0xaa7e, 0x4351,},
  {0x0235, 0x0137, 0x36b1, 0x336c,
   0xc66f, 0x8cdf, 0x80e9, 0x47c9, 0x93ba, 0x41a8,},
  {0x50f8, 0x25fb, 0xc76b, 0x6b71,
   0x3cbf, 0xa6d5, 0xffcf, 0x1f49, 0xc278, 0x40d3,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0xf020, 0xb59d, 0x2b70, 0xada8, 0x9dc5, 0x4069,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0400, 0xc9bf, 0x8e1b, 0x4034,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x2000, 0xbebc, 0x4019,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x9c40, 0x400c,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0xc800, 0x4005,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0xa000, 0x4002,},	/* 10**1 */
};

#else
static const unsigned short __etens[NTEN+1][NE] = {
{0xc94c,0x979a,0x8a20,0x5202,0xc460,0x7525,},/* 10**4096 */
{0xa74d,0x5de4,0xc53d,0x3b5d,0x9e8b,0x5a92,},/* 10**2048 */
{0x650d,0x0c17,0x8175,0x7586,0xc976,0x4d48,},
{0xcc65,0x91c6,0xa60e,0xa0ae,0xe319,0x46a3,},
{0xddbc,0xde8d,0x9df9,0xebfb,0xaa7e,0x4351,},
{0xc66f,0x8cdf,0x80e9,0x47c9,0x93ba,0x41a8,},
{0x3cbf,0xa6d5,0xffcf,0x1f49,0xc278,0x40d3,},
{0xf020,0xb59d,0x2b70,0xada8,0x9dc5,0x4069,},
{0x0000,0x0000,0x0400,0xc9bf,0x8e1b,0x4034,},
{0x0000,0x0000,0x0000,0x2000,0xbebc,0x4019,},
{0x0000,0x0000,0x0000,0x0000,0x9c40,0x400c,},
{0x0000,0x0000,0x0000,0x0000,0xc800,0x4005,},
{0x0000,0x0000,0x0000,0x0000,0xa000,0x4002,}, /* 10**1 */
};
#endif

#if NE == 10
static const unsigned short __emtens[NTEN + 1][NE] =
{
  {0x2030, 0xcffc, 0xa1c3, 0x8123,
   0x2de3, 0x9fde, 0xd2ce, 0x04c8, 0xa6dd, 0x0ad8,},	/* 10**-4096 */
  {0x8264, 0xd2cb, 0xf2ea, 0x12d4,
   0x4925, 0x2de4, 0x3436, 0x534f, 0xceae, 0x256b,},	/* 10**-2048 */
  {0xf53f, 0xf698, 0x6bd3, 0x0158,
   0x87a6, 0xc0bd, 0xda57, 0x82a5, 0xa2a6, 0x32b5,},
  {0xe731, 0x04d4, 0xe3f2, 0xd332,
   0x7132, 0xd21c, 0xdb23, 0xee32, 0x9049, 0x395a,},
  {0xa23e, 0x5308, 0xfefb, 0x1155,
   0xfa91, 0x1939, 0x637a, 0x4325, 0xc031, 0x3cac,},
  {0xe26d, 0xdbde, 0xd05d, 0xb3f6,
   0xac7c, 0xe4a0, 0x64bc, 0x467c, 0xddd0, 0x3e55,},
  {0x2a20, 0x6224, 0x47b3, 0x98d7,
   0x3f23, 0xe9a5, 0xa539, 0xea27, 0xa87f, 0x3f2a,},
  {0x0b5b, 0x4af2, 0xa581, 0x18ed,
   0x67de, 0x94ba, 0x4539, 0x1ead, 0xcfb1, 0x3f94,},
  {0xbf71, 0xa9b3, 0x7989, 0xbe68,
   0x4c2e, 0xe15b, 0xc44d, 0x94be, 0xe695, 0x3fc9,},
  {0x3d4d, 0x7c3d, 0x36ba, 0x0d2b,
   0xfdc2, 0xcefc, 0x8461, 0x7711, 0xabcc, 0x3fe4,},
  {0xc155, 0xa4a8, 0x404e, 0x6113,
   0xd3c3, 0x652b, 0xe219, 0x1758, 0xd1b7, 0x3ff1,},
  {0xd70a, 0x70a3, 0x0a3d, 0xa3d7,
   0x3d70, 0xd70a, 0x70a3, 0x0a3d, 0xa3d7, 0x3ff8,},
  {0xcccd, 0xcccc, 0xcccc, 0xcccc,
   0xcccc, 0xcccc, 0xcccc, 0xcccc, 0xcccc, 0x3ffb,},	/* 10**-1 */
};

#else
static const unsigned short __emtens[NTEN+1][NE] = {
{0x2de4,0x9fde,0xd2ce,0x04c8,0xa6dd,0x0ad8,}, /* 10**-4096 */
{0x4925,0x2de4,0x3436,0x534f,0xceae,0x256b,}, /* 10**-2048 */
{0x87a6,0xc0bd,0xda57,0x82a5,0xa2a6,0x32b5,},
{0x7133,0xd21c,0xdb23,0xee32,0x9049,0x395a,},
{0xfa91,0x1939,0x637a,0x4325,0xc031,0x3cac,},
{0xac7d,0xe4a0,0x64bc,0x467c,0xddd0,0x3e55,},
{0x3f24,0xe9a5,0xa539,0xea27,0xa87f,0x3f2a,},
{0x67de,0x94ba,0x4539,0x1ead,0xcfb1,0x3f94,},
{0x4c2f,0xe15b,0xc44d,0x94be,0xe695,0x3fc9,},
{0xfdc2,0xcefc,0x8461,0x7711,0xabcc,0x3fe4,},
{0xd3c3,0x652b,0xe219,0x1758,0xd1b7,0x3ff1,},
{0x3d71,0xd70a,0x70a3,0x0a3d,0xa3d7,0x3ff8,},
{0xcccd,0xcccc,0xcccc,0xcccc,0xcccc,0x3ffb,}, /* 10**-1 */
};
#endif

/* This routine will not return more than NDEC+1 digits. */
void __etoasc(short unsigned int * __restrict__ x,
		   char * __restrict__ string,
		   const int ndigits, const int outformat,
		   int* outexp)
{
long digit;
unsigned short y[NI], t[NI], u[NI], w[NI], equot[NI];
const unsigned short *r, *p;
const unsigned short *ten;
unsigned short sign;
int i, j, k, expon, ndigs;
char *s, *ss;
unsigned short m;

ndigs = ndigits;
#ifdef NANS
if( __eisnan(x) )
	{
	sprintf( string, " NaN " );
	expon = 9999;
	goto bxit;
	}
#endif
__emov( x, y ); /* retain external format */
if( y[NE-1] & 0x8000 )
	{
	sign = 0xffff;
	y[NE-1] &= 0x7fff;
	}
else
	{
	sign = 0;
	}
expon = 0;
ten = &__etens[NTEN][0];
__emov( __eone, t );
/* Test for zero exponent */
if( y[NE-1] == 0 )
	{
	for( k=0; k<NE-1; k++ )
		{
		if( y[k] != 0 )
			goto tnzro; /* denormalized number */
		}
	goto isone; /* legal all zeros */
	}
tnzro:

/* Test for infinity.
 */
if( y[NE-1] == 0x7fff )
	{
	if( sign )
		sprintf( string, " -Infinity " );
	else
		sprintf( string, " Infinity " );
	expon = 9999;
	goto bxit;
	}

/* Test for exponent nonzero but significand denormalized.
 * This is an error condition.
 */
if( (y[NE-1] != 0) && ((y[NE-2] & 0x8000) == 0) )
	{
	mtherr( "etoasc", DOMAIN );
	sprintf( string, "NaN" );
	expon = 9999;
	goto bxit;
	}

/* Compare to 1.0 */
i = __ecmp( __eone, y );
if( i == 0 )
	goto isone;

if( i < 0 )
	{ /* Number is greater than 1 */
/* Convert significand to an integer and strip trailing decimal zeros. */
	__emov( y, u );
	u[NE-1] = EXONE + NBITS - 1;

	p = &__etens[NTEN-4][0];
	m = 16;
do
	{
	__ediv( p, u, t);
	__efloor( t, w );
	for( j=0; j<NE-1; j++ )
		{
		if( t[j] != w[j] )
			goto noint;
		}
	__emov( t, u );
	expon += (int )m;
noint:
	p += NE;
	m >>= 1;
	}
while( m != 0 );

/* Rescale from integer significand */
	u[NE-1] += y[NE-1] - (unsigned int )(EXONE + NBITS - 1);
	__emov( u, y );
/* Find power of 10 */
	__emov( __eone, t );
	m = MAXP;
	p = &__etens[0][0];
	while( __ecmp( ten, u ) <= 0 )
		{
		if( __ecmp( p, u ) <= 0 )
			{
			__ediv( p, u, u );
			__emul( p, t, t );
			expon += (int )m;
			}
		m >>= 1;
		if( m == 0 )
			break;
		p += NE;
		}
	}
else
	{ /* Number is less than 1.0 */
/* Pad significand with trailing decimal zeros. */
	if( y[NE-1] == 0 )
		{
		while( (y[NE-2] & 0x8000) == 0 )
			{
			__emul( ten, y, y );
			expon -= 1;
			}
		}
	else
		{
		__emovi( y, w );
		for( i=0; i<NDEC+1; i++ )
			{
			if( (w[NI-1] & 0x7) != 0 )
				break;
/* multiply by 10 */
			__emovz( w, u );
			__eshdn1( u );
			__eshdn1( u );
			__eaddm( w, u );
			u[1] += 3;
			while( u[2] != 0 )
				{
				__eshdn1(u);
				u[1] += 1;
				}
			if( u[NI-1] != 0 )
				break;
			if( __eone[NE-1] <= u[1] )
				break;
			__emovz( u, w );
			expon -= 1;
			}
		__emovo( w, y );
		}
	k = -MAXP;
	p = &__emtens[0][0];
	r = &__etens[0][0];
	__emov( y, w );
	__emov( __eone, t );
	while( __ecmp( __eone, w ) > 0 )
		{
		if( __ecmp( p, w ) >= 0 )
			{
			__emul( r, w, w );
			__emul( r, t, t );
			expon += k;
			}
		k /= 2;
		if( k == 0 )
			break;
		p += NE;
		r += NE;
		}
	__ediv( t, __eone, t );
	}
isone:
/* Find the first (leading) digit. */
__emovi( t, w );
__emovz( w, t );
__emovi( y, w );
__emovz( w, y );
__eiremain( t, y, equot);
digit = equot[NI-1];
while( (digit == 0) && (__eiszero(y) == 0) )
	{
	__eshup1( y );
	__emovz( y, u );
	__eshup1( u );
	__eshup1( u );
	__eaddm( u, y );
	__eiremain( t, y, equot);
	digit = equot[NI-1];
	expon -= 1;
	}
s = string;
if( sign )
	*s++ = '-';
else
	*s++ = ' ';
/* Examine number of digits requested by caller. */
if( outformat == 3 )
        ndigs += expon;
/*
else if( ndigs < 0 )
        ndigs = 0;
*/
if( ndigs > NDEC )
	ndigs = NDEC;
if( digit == 10 )
	{
	*s++ = '1';
	*s++ = '.';
	if( ndigs > 0 )
		{
		*s++ = '0';
		ndigs -= 1;
		}
	expon += 1;
	if( ndigs < 0 )
	        {
	        ss = s;
	        goto doexp;
	        }
	}
else
	{
	*s++ = (char )digit + '0';
	*s++ = '.';
	}
/* Generate digits after the decimal point. */
for( k=0; k<=ndigs; k++ )
	{
/* multiply current number by 10, without normalizing */
	__eshup1( y );
	__emovz( y, u );
	__eshup1( u );
	__eshup1( u );
	__eaddm( u, y );
	__eiremain( t, y, equot);
	*s++ = (char )equot[NI-1] + '0';
	}
digit = equot[NI-1];
--s;
ss = s;
/* round off the ASCII string */
if( digit > 4 )
	{
/* Test for critical rounding case in ASCII output. */
	if( digit == 5 )
		{
		if( __eiiszero(y) == 0 )
			goto roun;	/* round to nearest */
		if( (*(s-1) & 1) == 0 )
			goto doexp;	/* round to even */
		}
/* Round up and propagate carry-outs */
roun:
	--s;
	k = *s & 0x7f;
/* Carry out to most significant digit? */
	if( ndigs < 0 )
		{
	        /* This will print like "1E-6". */
		*s = '1';

		expon += 1;
		goto doexp;
		}
	else if( k == '.' )
		{
		--s;
		k = *s;
		k += 1;
		*s = (char )k;
/* Most significant digit carries to 10? */
		if( k > '9' )
			{
			expon += 1;
			*s = '1';
			}
		goto doexp;
		}
/* Round up and carry out from less significant digits */
	k += 1;
	*s = (char )k;
	if( k > '9' )
		{
		*s = '0';
		goto roun;
		}
	}
doexp:
#if defined (__GO32__)  || defined (__MINGW32__) 
if( expon >= 0 )
	sprintf( ss, "e+%02d", expon );
else
	sprintf( ss, "e-%02d", -expon );
#else
	sprintf( ss, "E%d", expon );
#endif
bxit:

if (outexp)
  *outexp =  expon;
}


/* FIXME: Not thread safe */ 
static char outstr[128];

 char *
_IO_ldtoa(long double d, int mode, int ndigits, int *decpt,
	  int *sign, char **rve)
{
unsigned short e[NI];
char *s, *p;
int k;
int outexpon = 0;

union
  {
    unsigned short int us[6];
    long double ld;
  } xx;
xx.ld = d; 
__e64toe(xx.us, e );
if( __eisneg(e) )
        *sign = 1;
else
        *sign = 0;
/* Mode 3 is "f" format.  */
if( mode != 3 )
        ndigits -= 1;
/* Mode 0 is for %.999 format, which is supposed to give a
   minimum length string that will convert back to the same binary value.
   For now, just ask for 20 digits which is enough but sometimes too many.  */
if( mode == 0 )
        ndigits = 20;
/* This sanity limit must agree with the corresponding one in etoasc, to
   keep straight the returned value of outexpon.  */
if( ndigits > NDEC )
        ndigits = NDEC;

__etoasc( e, outstr, ndigits, mode, &outexpon );
s =  outstr;
if( __eisinf(e) || __eisnan(e) )
        {
        *decpt = 9999;
        goto stripspaces;
        }
*decpt = outexpon + 1;

/* Transform the string returned by etoasc into what the caller wants.  */

/* Look for decimal point and delete it from the string. */
s = outstr;
while( *s != '\0' )
        {
        if( *s == '.' )
               goto yesdecpt;
        ++s;
        }
goto nodecpt;

yesdecpt:

/* Delete the decimal point.  */
while( *s != '\0' )
        {
        *s = *(s+1);
        ++s;
        }

nodecpt:

/* Back up over the exponent field. */
while( *s != 'E' && *s != 'e' && s > outstr)
        --s;
*s = '\0';

stripspaces:

/* Strip leading spaces and sign. */
p = outstr;
while( *p == ' ' || *p == '-')
        ++p;

/* Find new end of string.  */
s = outstr;
while( (*s++ = *p++) != '\0' )
        ;
--s;

/* Strip trailing zeros.  */
if( mode == 2 )
        k = 1;
else if( ndigits > outexpon )
        k = ndigits;
else
        k = outexpon;

while( *(s-1) == '0' && ((s - outstr) > k))
        *(--s) = '\0';

/* In f format, flush small off-scale values to zero.
   Rounding has been taken care of by etoasc. */
if( mode == 3 && ((ndigits + outexpon) < 0))
        {
        s = outstr;
        *s = '\0';
        *decpt = 0;
        }

if( rve )
        *rve = s;
return outstr;
}

void
_IO_ldtostr(long double *x, char *string, int ndigs, int flags, char fmt)
{
unsigned short w[NI];
char *t, *u;
int outexpon = 0;
int outformat = -1;
char dec_sym = *(localeconv()->decimal_point);

__e64toe( (unsigned short *)x, w );
__etoasc( w, string, ndigs, outformat, &outexpon );

if( ndigs == 0 && flags == 0 )
	{
	/* Delete the decimal point unless alternate format.  */
	t = string;	
	while( *t != '.' )
		++t;
	u = t +  1;
	while( *t != '\0' )
		*t++ = *u++;
	}
if (*string == ' ')
	{
	t = string;	
	u = t + 1;
	while( *t != '\0' )
		*t++ = *u++;
	}
if (fmt == 'E')
	{
	t = string;	
	while( *t != 'e' )
		++t;
	*t = 'E';
	}
if (dec_sym != '.')
  {
    t = string;
    while (*t != '.')
      ++t;
    *t = dec_sym;
  }
}

#endif /* USE_LDTOA */
