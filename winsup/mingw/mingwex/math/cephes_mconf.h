#include <math.h>
#include <errno.h>

/* constants used by cephes functions */

#define MAXNUML 1.189731495357231765021263853E4932L
#define MAXLOGL	1.1356523406294143949492E4L
#define MINLOGL	-1.13994985314888605586758E4L
#define LOGE2L	6.9314718055994530941723E-1L
#define LOG2EL	1.4426950408889634073599E0L
#define PIL	3.1415926535897932384626L
#define PIO2L	1.5707963267948966192313L
#define PIO4L	7.8539816339744830961566E-1L

#define isfinitel isfinite
#define isinfl isinf
#define isnanl isnan
#define signbitl signbit


#define IBMPC 1
#define ANSIPROT 1
#define MINUSZERO 1
#define INFINITIES 1
#define NANS 1
#define DENORMAL 1
#define NEGZEROL (-0.0L)
extern long double __INFL;
#define INFINITYL (__INFL)
extern long double __QNANL;
#define NANL (__QNANL)
#define VOLATILE
#define mtherr(fname, code) 
#define XPD 0,

#ifdef _CEPHES_USE_ERRNO
#define _SET_ERRNO(x) errno = (x)
#else
#define _SET_ERRNO(x)
#endif
/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1988, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


/*							polevll.c
 *							p1evll.c
 *
 *	Evaluate polynomial
 *
 *
 *
 * SYNOPSIS:
 *
 * int N;
 * long double x, y, coef[N+1], polevl[];
 *
 * y = polevll( x, coef, N );
 *
 *
 *
 * DESCRIPTION:
 *
 * Evaluates polynomial of degree N:
 *
 *                     2          N
 * y  =  C  + C x + C x  +...+ C x
 *        0    1     2          N
 *
 * Coefficients are stored in reverse order:
 *
 * coef[0] = C  , ..., coef[N] = C  .
 *            N                   0
 *
 *  The function p1evll() assumes that coef[N] = 1.0 and is
 * omitted from the array.  Its calling arguments are
 * otherwise the same as polevll().
 *
 *
 * SPEED:
 *
 * In the interest of speed, there are no checks for out
 * of bounds arithmetic.  This routine is used by most of
 * the functions in the library.  Depending on available
 * equipment features, the user may wish to rewrite the
 * program in microcode or assembly language.
 *
 */

/* Polynomial evaluator:
 *  P[0] x^n  +  P[1] x^(n-1)  +  ...  +  P[n]
 */
static  __inline__ long double polevll( x, p, n )
long double x;
const void *p;
int n;
{
register long double y;
register long double *P = (long double *)p;

y = *P++;
do
	{
	y = y * x + *P++;
	}
while( --n );
return(y);
}



/* Polynomial evaluator:
 *  x^n  +  P[0] x^(n-1)  +  P[1] x^(n-2)  +  ...  +  P[n]
 */
static __inline__ long double p1evll( x, p, n )
long double x;
const void *p;
int n;
{
register long double y;
register long double *P = (long double *)p;

n -= 1;
y = x + *P++;
do
	{
	y = y * x + *P++;
	}
while( --n );
return( y );
}

