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


/* Traditional/XOPEN math constants (double precison) */
#ifndef __STRICT_ANSI__
#define M_E		2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440
#endif

/* These are also defined in Mingw float.h; needed here as well to work 
   around GCC build issues.  */
#ifndef	__STRICT_ANSI__
#ifndef __MINGW_FPCLASS_DEFINED
#define __MINGW_FPCLASS_DEFINED 1
/* IEEE 754 classication */
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
extern double*	_imp___HUGE;
#define	HUGE_VAL	(*_imp___HUGE)
#else
/* CRTDLL */
extern double*	_imp___HUGE_dll;
#define	HUGE_VAL	(*_imp___HUGE_dll)
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
extern __inline__  double fabs (double x)
{
  double res;
  __asm__ ("fabs;" : "=t" (res) : "0" (x));
  return res;
}
double	ldexp (double, int);
double	frexp (double, int*);
double	modf (double, double*);
double	fmod (double, double);


#ifndef	__STRICT_ANSI__

/* Complex number (for cabs). This really belongs in complex.h */
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


/*
 * Non-underscored versions of non-ANSI functions.
 * These reside in liboldnames.a.
 */

#if !defined (_NO_OLDNAMES)

double cabs (struct _complex);
double j0 (double);
double j1 (double);
double jn (int, double);
double y0 (double);
double y1 (double);
double yn (int, double);

double chgsign	(double);
double scalb (double, long);
int finite (double);
int fpclass (double);

#endif	/* Not _NO_OLDNAMES */

#endif /* __STRICT_ANSI__ */


#ifndef __NO_ISOCEXT
#if (defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
	|| !defined  __STRICT_ANSI__  || defined __GLIBCPP__

#define NAN (0.0F/0.0F)
#define HUGE_VALF (1.0F/0.0F)
#define HUGE_VALL (1.0L/0.0L)
#define INFINITY (1.0F/0.0F)


/* 7.12.3.1 */
/*
   Return values for fpclassify.
   These are based on Intel x87 fpu condition codes
   in the high byte of status word and differ from
   the return values for MS IEEE 754 extension _fpclass()
*/
#define FP_NAN		0x0100
#define FP_NORMAL	0x0400
#define FP_INFINITE	(FP_NAN | FP_NORMAL)
#define FP_ZERO		0x4000
#define FP_SUBNORMAL	(FP_NORMAL | FP_ZERO)
/* 0x0200 is signbit mask */


/*
  We can't inline float or double, because we want to ensure truncation
  to semantic type before classification. 
  (A normal long double value might become subnormal when 
  converted to double, and zero when converted to float.)
*/

extern int __fpclassifyf (float);
extern int __fpclassify (double);

extern __inline__ int __fpclassifyl (long double x){
  unsigned short sw;
  __asm__ ("fxam; fstsw %%ax;" : "=a" (sw): "t" (x));
  return sw & (FP_NAN | FP_NORMAL | FP_ZERO );
}

#define fpclassify(x) (sizeof (x) == sizeof (float) ? __fpclassifyf (x)	  \
		       : sizeof (x) == sizeof (double) ? __fpclassify (x) \
		       : __fpclassifyl (x))

/* 7.12.3.2 */
#define isfinite(x) ((fpclassify(x) & FP_NAN) == 0)

/* 7.12.3.3 */
#define isinf(x) (fpclassify(x) == FP_INFINITE)

/* 7.12.3.4 */
/* We don't need to worry about trucation here:
   A NaN stays a NaN. */

extern __inline__ int __isnan (double _x)
{
  unsigned short sw;
  __asm__ ("fxam;"
	   "fstsw %%ax": "=a" (sw) : "t" (_x));
  return (sw & (FP_NAN | FP_NORMAL | FP_INFINITE | FP_ZERO | FP_SUBNORMAL))
    == FP_NAN;
}

extern __inline__ int __isnanf (float _x)
{
  unsigned short sw;
  __asm__ ("fxam;"
	    "fstsw %%ax": "=a" (sw) : "t" (_x));
  return (sw & (FP_NAN | FP_NORMAL | FP_INFINITE | FP_ZERO | FP_SUBNORMAL))
    == FP_NAN;
}

extern __inline__ int __isnanl (long double _x)
{
  unsigned short sw;
  __asm__ ("fxam;"
	    "fstsw %%ax": "=a" (sw) : "t" (_x));
  return (sw & (FP_NAN | FP_NORMAL | FP_INFINITE | FP_ZERO | FP_SUBNORMAL))
    == FP_NAN;
}


#define isnan(x) (sizeof (x) == sizeof (float) ? __isnanf (x)	\
		  : sizeof (x) == sizeof (double) ? __isnan (x)	\
		  : __isnanl (x))

/* 7.12.3.5 */
#define isnormal(x) (fpclassify(x) == FP_NORMAL)

/* 7.12.3.6 The signbit macro */
extern __inline__ int __signbit (double x) {
  unsigned short stw;
  __asm__ ( "fxam; fstsw %%ax;": "=a" (stw) : "t" (x));
  return stw & 0x0200;
}

extern  __inline__ int __signbitf (float x) {
  unsigned short stw;
  __asm__ ("fxam; fstsw %%ax;": "=a" (stw) : "t" (x));
  return stw & 0x0200;
}
extern  __inline__ int __signbitl (long double x) {
  unsigned short stw;
  __asm__ ("fxam; fstsw %%ax;": "=a" (stw) : "t" (x));
  return stw & 0x0200;
}


#define signbit(x) (sizeof (x) == sizeof (float) ? __signbitf (x)	\
		    : sizeof (x) == sizeof (double) ? __signbit (x)	\
		    : __signbitl (x))

/* 7.12.4 Trigonometric functions: Double in C89 */
extern float sinf (float);
extern long double sinl (long double);

extern float cosf (float);
extern long double cosl (long double);

extern float tanf (float);
extern long double tanl (long double);

extern float asinf (float);
extern long double asinl (long double);

extern float acosf (float);
extern long double acosl (long double);

extern float atanf (float);
extern long double atanl (long double);

extern float atan2f (float, float);
extern long double atan2l (long double, long double);

/* 7.12.5 Hyperbolic functions: Double in C89  */
extern __inline__ float sinhf (float x)
  {return (float) sinh (x);}
extern long double sinhl (long double);

extern __inline__ float coshf (float x)
  {return (float) cosh (x);}
extern long double coshl (long double);

extern __inline__ float tanhf (float x)
  {return (float) tanh (x);}
extern long double tanhl (long double);

/*
 * TODO: asinh, acosh, atanh
 */ 

/* 7.12.6.1 Double in C89 */
extern __inline__ float expf (float x)
  {return (float) exp (x);}
extern long double expl (long double);

/* 7.12.6.2 */
extern double exp2(double);
extern float exp2f(float);
extern long double exp2l(long double);

/* 7.12.6.3 The expm1 functions: TODO */

/* 7.12.6.4 Double in C89 */
extern __inline__ float frexpf (float x, int* expn)
  {return (float) frexp (x, expn);}
extern long double frexpl (long double, int*);

/* 7.12.6.5 */
#define FP_ILOGB0 ((int)0x80000000)
#define FP_ILOGBNAN ((int)0x80000000)
extern int ilogb (double);
extern int ilogbf (float);
extern int ilogbl (long double);

/* 7.12.6.6  Double in C89 */
extern __inline__ float ldexpf (float x, int expn)
  {return (float) ldexp (x, expn);}
extern long double ldexpl (long double, int);

/* 7.12.6.7 Double in C89 */
extern float logf (float);
extern long double logl (long double);

/* 7.12.6.8 Double in C89 */
extern float log10f (float);
extern long double log10l (long double);

/* 7.12.6.9 */
extern double log1p(double);
extern float log1pf(float);
extern long double log1pl(long double);

/* 7.12.6.10 */
extern double log2 (double);
extern float log2f (float);
extern long double log2l (long double);

/* 7.12.6.11 */
extern double logb (double);
extern float logbf (float);
extern long double logbl (long double);

extern __inline__ double logb (double x)
{
  double res;
  __asm__ ("fxtract\n\t"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}

extern __inline__ float logbf (float x)
{
  float res;
  __asm__ ("fxtract\n\t"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}

extern __inline__ long double logbl (long double x)
{
  long double res;
  __asm__ ("fxtract\n\t"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}

/* 7.12.6.12  Double in C89 */
extern float modff (float, float*);
extern long double modfl (long double, long double*);

/* 7.12.6.13 */
extern double scalbn (double, int);
extern float scalbnf (float, int);
extern long double scalbnl (long double, int);

extern double scalbln (double, long);
extern float scalblnf (float, long);
extern long double scalblnl (long double, long);

/* 7.12.7.1 */
/* Implementations adapted from Cephes versions */ 
extern double cbrt (double);
extern float cbrtf (float);
extern long double cbrtl (long double);

/* 7.12.7.2 The fabs functions: Double in C89 */
extern __inline__ float fabsf (float x)
{
  float res;
  __asm__ ("fabs;" : "=t" (res) : "0" (x));
  return res;
}

extern __inline__ long double fabsl (long double x)
{
  long double res;
  __asm__ ("fabs;" : "=t" (res) : "0" (x));
  return res;
}

/* 7.12.7.3  */
extern double hypot (double, double); /* in libmoldname.a */
extern __inline__ float hypotf (float x, float y)
  { return (float) hypot (x, y);}
extern long double hypotl (long double, long double);

/* 7.12.7.4 The pow functions. Double in C89 */
extern __inline__ float powf (float x, float y)
  {return (float) pow (x, y);}
extern long double powl (long double, long double);

/* 7.12.7.5 The sqrt functions. Double in C89. */
extern float sqrtf (float);
extern long double sqrtl (long double);

/* 7.12.8.1 The erf functions  */
extern double erf (double);
extern float erff (float);
/* TODO
extern long double erfl (long double);
*/ 

/* 7.12.8.2 The erfc functions  */
extern double erfc (double);
extern float erfcf (float);
/* TODO
extern long double erfcl (long double);
*/ 

/* 7.12.8.3 The lgamma functions */

extern double lgamma (double);
extern float lgammaf (float);
extern long double lgammal (long double);

/* 77.12.8.4 The tgamma functions */

extern double tgamma (double);
extern float tgammaf (float);
extern long double tgammal (long double);

/* 7.12.9.1 Double in C89 */
extern float ceilf (float);
extern long double ceill (long double);

/* 7.12.9.2 Double in C89 */
extern float floorf (float);
extern long double floorl (long double);

/* 7.12.9.3 */
extern double nearbyint ( double);
extern float nearbyintf (float);
extern long double nearbyintl (long double);

/* 7.12.9.4 */
/* round, using fpu control word settings */
extern __inline__ double rint (double x)
{
  double retval;
  __asm__ ("frndint;": "=t" (retval) : "0" (x));
  return retval;
}

extern __inline__ float rintf (float x)
{
  float retval;
  __asm__ ("frndint;" : "=t" (retval) : "0" (x) );
  return retval;
}

extern __inline__ long double rintl (long double x)
{
  long double retval;
  __asm__ ("frndint;" : "=t" (retval) : "0" (x) );
  return retval;
}

/* 7.12.9.5 */
extern __inline__ long lrint (double x) 
{
  long retval;  
  __asm__ __volatile__							      \
    ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

extern __inline__ long lrintf (float x) 
{
  long retval;
  __asm__ __volatile__							      \
    ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

extern __inline__ long lrintl (long double x) 
{
  long retval;
  __asm__ __volatile__							      \
    ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

extern __inline__ long long llrint (double x) 
{
  long long retval;
  __asm__ __volatile__							      \
    ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

extern __inline__ long long llrintf (float x) 
{
  long long retval;
  __asm__ __volatile__							      \
    ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

extern __inline__ long long llrintl (long double x) 
{
  long long retval;
  __asm__ __volatile__							      \
    ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

/* 7.12.9.6 */
/* round away from zero, regardless of fpu control word settings */
extern double round (double);
extern float roundf (float);
extern long double roundl (long double);

/* 7.12.9.7  */
extern long lround (double);
extern long lroundf (float);
extern long lroundl (long double);

extern long long llround (double);
extern long long llroundf (float);
extern long long llroundl (long double);

/* 7.12.9.8 */
/* round towards zero, regardless of fpu control word settings */
extern double trunc (double);
extern float truncf (float);
extern long double truncl (long double);

/* 7.12.10.1 Double in C89 */
extern float fmodf (float, float);
extern long double fmodl (long double, long double);

/* 7.12.10.2 */ 
extern double remainder (double, double);
extern float remainderf (float, float);
extern long double remainderl (long double, long double);

/* 7.12.10.3 */
extern double remquo(double, double, int *);
extern float remquof(float, float, int *);
extern long double remquol(long double, long double, int *);

/* 7.12.11.1 */
extern double copysign (double, double); /* in libmoldname.a */
extern float copysignf (float, float);
extern long double copysignl (long double, long double);

/* 7.12.11.2 Return a NaN */
extern double nan(const char *tagp);
extern float nanf(const char *tagp);
extern long double nanl(const char *tagp);

#ifndef __STRICT_ANSI__
#define _nan() nan("")
#define _nanf() nanf("")
#define _nanl() nanl("")
#endif

/* 7.12.11.3 */
extern double nextafter (double, double); /* in libmoldname.a */
extern float nextafterf (float, float);
/* TODO: Not yet implemented */
/* extern long double nextafterl (long double, long double); */

/* 7.12.11.4 The nexttoward functions: TODO */

/* 7.12.12.1 */
/*  x > y ? (x - y) : 0.0  */
extern double fdim (double x, double y);
extern float fdimf (float x, float y);
extern long double fdiml (long double x, long double y);

/* fmax and fmin.
   NaN arguments are treated as missing data: if one argument is a NaN
   and the other numeric, then these functions choose the numeric
   value. */

/* 7.12.12.2 */
extern double fmax  (double, double);
extern float fmaxf (float, float);
extern long double fmaxl (long double, long double);

/* 7.12.12.3 */
extern double fmin (double, double);
extern float fminf (float, float);
extern long double fminl (long double, long double);

/* 7.12.13.1 */
/* return x * y + z as a ternary op */ 
extern double fma (double, double, double);
extern float fmaf (float, float, float);
extern long double fmal (long double, long double, long double);


/* 7.12.14 */
/* 
 *  With these functions, comparisons involving quiet NaNs set the FP
 *  condition code to "unordered".  The IEEE floating-point spec
 *  dictates that the result of floating-point comparisons should be
 *  false whenever a NaN is involved, with the exception of the != op, 
 *  which always returns true: yes, (NaN != NaN) is true).
 */

#if __GNUC__ >= 3

#define isgreater(x, y) __builtin_isgreater(x, y)
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
#define isless(x, y) __builtin_isless(x, y)
#define islessequal(x, y) __builtin_islessequal(x, y)
#define islessgreater(x, y) __builtin_islessgreater(x, y)
#define isunordered(x, y) __builtin_isunordered(x, y)

#else
/*  helper  */
extern  __inline__ int
__fp_unordered_compare (long double x, long double y){
  unsigned short retval;
  __asm__ ("fucom %%st(1);"
	   "fnstsw;": "=a" (retval) : "t" (x), "u" (y));
  return retval;
}

#define isgreater(x, y) ((__fp_unordered_compare(x, y) \
			   & 0x4500) == 0)
#define isless(x, y) ((__fp_unordered_compare (y, x) \
                       & 0x4500) == 0)
#define isgreaterequal(x, y) ((__fp_unordered_compare (x, y) \
                               & FP_INFINITE) == 0)
#define islessequal(x, y) ((__fp_unordered_compare(y, x) \
			    & FP_INFINITE) == 0)
#define islessgreater(x, y) ((__fp_unordered_compare(x, y) \
			      & FP_SUBNORMAL) == 0)
#define isunordered(x, y) ((__fp_unordered_compare(x, y) \
			    & 0x4500) == 0x4500)

#endif


#endif /* __STDC_VERSION__ >= 199901L */
#endif /* __NO_ISOCEXT */


#ifdef __cplusplus
}
#endif
#endif	/* Not RC_INVOKED */


#endif	/* Not _MATH_H_ */
