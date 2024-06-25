/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef  _MATH_H_

#define  _MATH_H_

#include <sys/cdefs.h>
#include <ieeefp.h>
#include "_ansi.h"

_BEGIN_STD_C

/* Natural log of 2 */
#define _M_LN2        0.693147180559945309417
#define _M_LN2_LD     0.693147180559945309417232121458176568l

#if __GNUC_PREREQ (3, 3) || defined(__clang__) || defined(__COMPCERT__)
 /* gcc >= 3.3 implicitly defines builtins for HUGE_VALx values.  */

# ifndef HUGE_VAL
#  define HUGE_VAL (__builtin_huge_val())
# endif

# ifndef HUGE_VALF
#  define HUGE_VALF (__builtin_huge_valf())
# endif

# ifndef HUGE_VALL
#  define HUGE_VALL (__builtin_huge_vall())
# endif

# ifndef INFINITY
#  define INFINITY (__builtin_inff())
# endif

# ifndef NAN
#  define NAN (__builtin_nanf(""))
# endif

#else /* !gcc >= 3.3  */

 /*      No builtins.  Use fixed defines instead.  (All 3 HUGE plus the INFINITY
  * and NAN macros are required to be constant expressions.  Using a variable--
  * even a static const--does not meet this requirement, as it cannot be
  * evaluated at translation time.)
  *      The infinities are done using numbers that are far in excess of
  * something that would be expected to be encountered in a floating-point
  * implementation.  (A more certain way uses values from float.h, but that is
  * avoided because system includes are not supposed to include each other.)
  *      This method might produce warnings from some compilers.  (It does in
  * newer GCCs, but not for ones that would hit this #else.)  If this happens,
  * please report details to the Newlib mailing list.  */

 #ifndef HUGE_VAL
  #define HUGE_VAL (1.0e999999999)
 #endif

 #ifndef HUGE_VALF
  #define HUGE_VALF (1.0e999999999F)
 #endif

 #if !defined(HUGE_VALL)  &&  defined(_HAVE_LONG_DOUBLE)
  #define HUGE_VALL (1.0e999999999L)
 #endif

 #if !defined(INFINITY)
  #define INFINITY (HUGE_VALF)
 #endif

 #if !defined(NAN)
  #if defined(__GNUC__)  &&  defined(__cplusplus)
    /* Exception:  older g++ versions warn about the divide by 0 used in the
     * normal case (even though older gccs do not).  This trick suppresses the
     * warning, but causes errors for plain gcc, so is only used in the one
     * special case.  */
    static const union { __ULong __i[1]; float __d; } __Nanf = {0x7FC00000};
    #define NAN (__Nanf.__d)
  #else
    #define NAN (0.0F/0.0F)
  #endif
 #endif

#endif /* !gcc >= 3.3  */

extern double atan (double);
extern double cos (double);
extern double sin (double);
extern double tan (double);
extern double tanh (double);
extern double frexp (double, int *);
extern double modf (double, double *);
extern double ceil (double);
extern double fabs (double);
extern double floor (double);

extern double acos (double);
extern double asin (double);
extern double atan2 (double, double);
extern double cosh (double);
extern double sinh (double);
extern double exp (double);
extern double ldexp (double, int);
extern double log (double);
extern double log10 (double);
extern double pow (double, double);
extern double sqrt (double);
extern double fmod (double, double);

#if __MISC_VISIBLE
extern int finite (double);
extern int finitef (float);
extern int isinff (float);
extern int isnanf (float);
#if defined(_HAVE_LONG_DOUBLE)
extern int finitel (long double);
extern int isinfl (long double);
extern int isnanl (long double);
#endif
#if !defined(__cplusplus) || __cplusplus < 201103L
extern int isinf (double);
#endif
#endif /* __MISC_VISIBLE */
#if (__MISC_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 600)) \
  && (!defined(__cplusplus) || __cplusplus < 201103L)
extern int isnan (double);
#endif

#if __ISO_C_VISIBLE >= 1999
/* ISO C99 types and macros. */

/* FIXME:  FLT_EVAL_METHOD should somehow be gotten from float.h (which is hard,
 * considering that the standard says the includes it defines should not
 * include other includes that it defines) and that value used.  (This can be
 * solved, but autoconf has a bug which makes the solution more difficult, so
 * it has been skipped for now.)  */
#if !defined(FLT_EVAL_METHOD) && defined(__FLT_EVAL_METHOD__)
  #define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
  #define __TMP_FLT_EVAL_METHOD
#endif /* FLT_EVAL_METHOD */
#if defined FLT_EVAL_METHOD
/* FLT_EVAL_METHOD == 16 has meaning as defined in ISO/IEC TS 18661-3,
 * which provides non-compliant extensions to C and POSIX (by adding
 * additional positive values for FLT_EVAL_METHOD).  It effectively has
 * same meaning as the C99 and C11 definitions for value 0, while also
 * serving as a flag that the _Float16 (float16_t) type exists.
 *
 * FLT_EVAL_METHOD could be any number of bits of supported floating point
 * format (e.g. 32, 64, 128), but currently only AArch64 and few other targets
 * might define that as 16.  */
  #if (FLT_EVAL_METHOD == 0) \
      || (FLT_EVAL_METHOD == 16)
    typedef float  float_t;
    typedef double double_t;
   #elif FLT_EVAL_METHOD == 1
    typedef double float_t;
    typedef double double_t;
   #elif FLT_EVAL_METHOD == 2
    typedef long double float_t;
    typedef long double double_t;
   #else
    /* Implementation-defined.  Assume float_t and double_t have been
     * defined previously for this configuration (e.g. config.h). */

   /* If __DOUBLE_TYPE is defined (__FLOAT_TYPE is then supposed to be
      defined as well) float_t and double_t definition is suggested by
      an arch specific header.  */
   #ifdef __DOUBLE_TYPE
    typedef __DOUBLE_TYPE double_t;
    typedef __FLOAT_TYPE float_t;
   #endif
   /* Assume config.h has provided these types.  */
  #endif
#else
    /* Assume basic definitions.  */
    typedef float  float_t;
    typedef double double_t;
#endif
#if defined(__TMP_FLT_EVAL_METHOD)
  #undef FLT_EVAL_METHOD
#endif

#define FP_NAN         0
#define FP_INFINITE    1
#define FP_ZERO        2
#define FP_SUBNORMAL   3
#define FP_NORMAL      4

#ifndef FP_ILOGB0
# define FP_ILOGB0 (-__INT_MAX__)
#endif
#ifndef FP_ILOGBNAN
# define FP_ILOGBNAN __INT_MAX__
#endif

#ifndef MATH_ERRNO
# define MATH_ERRNO 1
#endif
#ifndef MATH_ERREXCEPT
# define MATH_ERREXCEPT 2
#endif
#ifndef math_errhandling
# ifdef _IEEE_LIBM
#  define _MATH_ERRHANDLING_ERRNO 0
# else
#  define _MATH_ERRHANDLING_ERRNO MATH_ERRNO
# endif
# ifdef _SUPPORTS_ERREXCEPT
#  define _MATH_ERRHANDLING_ERREXCEPT MATH_ERREXCEPT
# else
#  define _MATH_ERRHANDLING_ERREXCEPT 0
# endif
# define math_errhandling (_MATH_ERRHANDLING_ERRNO | _MATH_ERRHANDLING_ERREXCEPT)
#endif

/*
 * Specifies whether the target uses the snan/nan discriminator
 * definition from IEEE 754 2008 (top bit of significand is 1 for qNaN
 * and 0 for sNaN). This is set to zero in machine/math.h for targets
 * that don't do this (such as MIPS).
 */
#ifndef _IEEE_754_2008_SNAN
# define _IEEE_754_2008_SNAN 1
#endif

extern int __isinff (float);
extern int __isinfd (double);
extern int __isnanf (float);
extern int __isnand (double);
extern int __fpclassifyf (float);
extern int __fpclassifyd (double);
extern int __signbitf (float);
extern int __signbitd (double);
extern int __finite (double);
extern int __finitef (float);
#if defined(_HAVE_LONG_DOUBLE)
extern int __fpclassifyl (long double);
extern int __isinfl (long double);
extern int __isnanl (long double);
extern int __finitel (long double);
extern int __signbitl (long double);
#endif

/* Note: isinf and isnan were once functions in newlib that took double
 *       arguments.  C99 specifies that these names are reserved for macros
 *       supporting multiple floating point types.  Thus, they are
 *       now defined as macros.  Implementations of the old functions
 *       taking double arguments still exist for compatibility purposes
 *       (prototypes for them are earlier in this header).  */

/*
 * GCC bug 66462 raises INVALID exception when __builtin_fpclassify is
 * passed snan, so we cannot use it when building with snan support.
 * clang doesn't appear to have an option to control snan behavior, and
 * it's builtin fpclassify also raises INVALID for snan, so always use
 * our version for that.
 */
#if __GNUC_PREREQ (4, 4) && !defined(__SUPPORT_SNAN__) && !defined(__clang__)
  #define fpclassify(__x) (__builtin_fpclassify (FP_NAN, FP_INFINITE, \
						 FP_NORMAL, FP_SUBNORMAL, \
						 FP_ZERO, __x))
  #define isfinite(__x)	(__builtin_isfinite (__x))
  #define isinf(__x) (__builtin_isinf_sign (__x))
  #define isnan(__x) (__builtin_isnan (__x))
  #define isnormal(__x) (__builtin_isnormal (__x))
  #define issubnormal(__x) (__builtin_issubnormal (__x))
  #define iszero(__x) (__builtin_iszero(__x))
#else
#if defined(_HAVE_LONG_DOUBLE)
  #define fpclassify(__x)                                                 \
      ((sizeof(__x) == sizeof(float)  ? __fpclassifyf(__x)                \
        : (sizeof(__x) == sizeof(double)) ?  __fpclassifyd((double) (__x)) \
        : __fpclassifyl((long double) (__x))))
  #define isfinite(__x)                                           \
      ((sizeof(__x) == sizeof(float)) ? __finitef(__x)            \
       : (sizeof(__x) == sizeof(double)) ? __finite((double) (__x))     \
       : __finitel((long double) (__x)))
#else
  #define fpclassify(__x) \
	  ((sizeof(__x) == sizeof(float))  ? __fpclassifyf(__x) : \
	  __fpclassifyd((double) (__x)))
  #define isfinite(__x) ((sizeof(__x) == sizeof(float)) ?         \
                         __finitef(__x) : __finite((double) __x))
#endif
  #define isinf(__x) (fpclassify(__x) == FP_INFINITE)
  #define isnan(__x) (fpclassify(__x) == FP_NAN)
  #define isnormal(__x) (fpclassify(__x) == FP_NORMAL)
  #define issubnormal(__x) (fpclassify(__x) == FP_SUBNORMAL)
  #define iszero(__x) (fpclassify(__x) == FP_ZERO)
#endif

#define isfinitef(x) isfinite((float) (x))
#define isinff(x) isinf((float) (x))
#define isnanf(x) isnan((float) (x))
#define isnormalf(x) isnormal((float) (x))
#define iszerof(x) iszero((float) (x))

#ifndef iseqsig
int __iseqsigd(double x, double y);
int __iseqsigf(float x, float y);

#ifdef _HAVE_LONG_DOUBLE
int __iseqsigl(long double x, long double y);
#define iseqsig(__x,__y)                                                \
    ((sizeof(__x) == sizeof(float) && sizeof(__y) == sizeof(float)) ?   \
     __iseqsigf(__x, __y) :                                             \
     (sizeof(__x) == sizeof(double) && sizeof(__y) == sizeof(double)) ? \
     __iseqsigd(__x, __y) :                                             \
     __iseqsigl(__x, __y))
#else
#ifdef _DOUBLE_IS_32BITS
#define iseqsig(__x, __y) __iseqsigf((float)(__x), (float)(__y))
#else
#define iseqsig(__x,__y)                                                \
    ((sizeof(__x) == sizeof(float) && sizeof(__y) == sizeof(float)) ?   \
     __iseqsigf(__x, __y) : __iseqsigd(__x, __y))
#endif
#endif
#endif

#ifndef issignaling
int __issignalingf(float f);
int __issignaling(double d);

#if defined(_HAVE_LONG_DOUBLE)
int __issignalingl(long double d);
#define issignaling(__x)						\
	((sizeof(__x) == sizeof(float))  ? __issignalingf(__x) :	\
	 (sizeof(__x) == sizeof(double)) ? __issignaling ((double) (__x)) :	\
	 __issignalingl((long double) (__x)))
#else
#ifdef _DOUBLE_IS_32BITS
#define issignaling(__x) __issignalingf((float) (__x))
#else
#define issignaling(__x)						\
	((sizeof(__x) == sizeof(float))  ? __issignalingf(__x) :	\
	 __issignaling ((double) (__x)))
#endif /* _DOUBLE_IS_32BITS */
#endif /* _HAVE_LONG_DOUBLE */
#endif

#if __GNUC_PREREQ (4, 0)
  #if defined(_HAVE_LONG_DOUBLE)
    #define signbit(__x)							\
	    ((sizeof(__x) == sizeof(float))  ? __builtin_signbitf(__x) :	\
	     ((sizeof(__x) == sizeof(double)) ? __builtin_signbit ((double)(__x)) : \
	      __builtin_signbitl((long double)(__x))))
  #else
    #define signbit(__x)							\
	    ((sizeof(__x) == sizeof(float))  ? __builtin_signbitf(__x) :	\
	     __builtin_signbit ((double) (__x)))
  #endif
#else
  #if defined(_HAVE_LONG_DOUBLE)
    #define signbit(__x)							\
	    ((sizeof(__x) == sizeof(float))  ? __signbitf(__x) :	\
	     ((sizeof(__x) == sizeof(double)) ? __signbit ((double)(__x)) : \
	      __signbitl((long double)(__x))))
  #else
    #define signbit(__x) \
            ((sizeof(__x) == sizeof(float))  ?  __signbitf(__x) : \
                                        __signbitd((double) (__x)))
  #endif
#endif

#if __GNUC_PREREQ (2, 97) && !(defined(__riscv) && defined(__clang__))
#define isgreater(__x,__y)	(__builtin_isgreater (__x, __y))
#define isgreaterequal(__x,__y)	(__builtin_isgreaterequal (__x, __y))
#define isless(__x,__y)		(__builtin_isless (__x, __y))
#define islessequal(__x,__y)	(__builtin_islessequal (__x, __y))
#define islessgreater(__x,__y)	(__builtin_islessgreater (__x, __y))
#define isunordered(__x,__y)	(__builtin_isunordered (__x, __y))
#else
#define isgreater(x,y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x > __y);}))
#define isgreaterequal(x,y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x >= __y);}))
#define isless(x,y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x < __y);}))
#define islessequal(x,y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x <= __y);}))
#define islessgreater(x,y) \
          (__extension__ ({__typeof__(x) __x = (x); __typeof__(y) __y = (y); \
                           !isunordered(__x,__y) && (__x < __y || __x > __y);}))

#define isunordered(a,b) \
          (__extension__ ({__typeof__(a) __a = (a); __typeof__(b) __b = (b); \
                           fpclassify(__a) == FP_NAN || fpclassify(__b) == FP_NAN;}))
#endif

/* Non ANSI double precision functions.  */

extern double infinity (void);
extern double nan (const char *);
extern double copysign (double, double);
extern double logb (double);
extern int ilogb (double);

extern double asinh (double);
extern double cbrt (double);
extern double nextafter (double, double);
extern double rint (double);
extern double scalbn (double, int);
extern double scalb (double, double);
extern double getpayload(const double *x);
extern double significand (double);

extern double exp2 (double);
extern double scalbln (double, long int);
extern double tgamma (double);
extern double nearbyint (double);
extern long int lrint (double);
extern long long int llrint (double);
extern double round (double);
extern long int lround (double);
extern long long int llround (double);
extern double trunc (double);
extern double remquo (double, double, int *);
extern double fdim (double, double);
extern double fmax (double, double);
extern double fmin (double, double);
extern double fma (double, double, double);

extern double log1p (double);
extern double expm1 (double);

extern double acosh (double);
extern double atanh (double);
extern double remainder (double, double);
extern double gamma (double);
extern double lgamma (double);
extern double erf (double);
extern double erfc (double);
extern double log2 (double);
#if !defined(__cplusplus)
#define log2(x) (log (x) / _M_LN2)
#endif

extern double hypot (double, double);

/* Single precision versions of ANSI functions.  */

extern float atanf (float);
extern float cosf (float);
extern float sinf (float);
extern float tanf (float);
extern float tanhf (float);
extern float frexpf (float, int *);
extern float modff (float, float *);
extern float ceilf (float);
extern float fabsf (float);
extern float floorf (float);

extern float acosf (float);
extern float asinf (float);
extern float atan2f (float, float);
extern float coshf (float);
extern float sinhf (float);
extern float expf (float);
extern float ldexpf (float, int);
extern float logf (float);
extern float log10f (float);
extern float powf (float, float);
extern float sqrtf (float);
extern float fmodf (float, float);

/* Other single precision functions.  */

extern float exp2f (float);
extern float scalblnf (float, long int);
extern float tgammaf (float);
extern float nearbyintf (float);
extern long int lrintf (float);
extern long long int llrintf (float);
extern float roundf (float);
extern long int lroundf (float);
extern long long int llroundf (float);
extern float truncf (float);
extern float remquof (float, float, int *);
extern float fdimf (float, float);
extern float fmaxf (float, float);
extern float fminf (float, float);
extern float fmaf (float, float, float);

extern float infinityf (void);
extern float nanf (const char *);
extern float copysignf (float, float);
extern float logbf (float);
extern int ilogbf (float);

extern float asinhf (float);
extern float cbrtf (float);
extern float nextafterf (float, float);
extern float rintf (float);
extern float scalbnf (float, int);
extern float scalbf (float, float);
extern float log1pf (float);
extern float expm1f (float);
extern float getpayloadf(const float *x);
extern float significandf (float);

extern float acoshf (float);
extern float atanhf (float);
extern float remainderf (float, float);
extern float gammaf (float);
extern float lgammaf (float);
extern float erff (float);
extern float erfcf (float);
extern float log2f (float);
extern float hypotf (float, float);

#ifdef _HAVE_LONG_DOUBLE

/* These functions are always available for long double */

extern long double hypotl (long double, long double);
extern long double infinityl (void);
extern long double sqrtl (long double);
extern long double frexpl (long double, int *);
extern long double scalbnl (long double, int);
extern long double scalblnl (long double, long);
extern long double rintl (long double);
extern long int lrintl (long double);
extern long long int llrintl (long double);
extern int ilogbl (long double);
extern long double logbl (long double);
extern long double ldexpl (long double, int);
extern long double nearbyintl (long double);
extern long double ceill (long double);
extern long double fmaxl (long double, long double);
extern long double fminl (long double, long double);
extern long double roundl (long double);
extern long lroundl (long double);
extern long long int llroundl (long double);
extern long double truncl (long double);
extern long double nanl (const char *);
extern long double floorl (long double);
extern long double scalbl (long double, long double);
extern long double significandl(long double);
/* Compiler provides these */
extern long double fabsl (long double);
extern long double copysignl (long double, long double);

#ifdef _HAVE_LONG_DOUBLE_MATH
extern long double atanl (long double);
extern long double cosl (long double);
extern long double sinl (long double);
extern long double tanl (long double);
extern long double tanhl (long double);
extern long double modfl (long double, long double *);
extern long double log1pl (long double);
extern long double expm1l (long double);
extern long double acosl (long double);
extern long double asinl (long double);
extern long double atan2l (long double, long double);
extern long double coshl (long double);
extern long double sinhl (long double);
extern long double expl (long double);
extern long double logl (long double);
extern long double log10l (long double);
extern long double powl (long double, long double);
extern long double fmodl (long double, long double);
extern long double asinhl (long double);
extern long double cbrtl (long double);
extern long double nextafterl (long double, long double);
extern float nexttowardf (float, long double);
extern double nexttoward (double, long double);
extern long double nexttowardl (long double, long double);
extern long double log2l (long double);
extern long double exp2l (long double);
extern long double tgammal (long double);
extern long double remquol (long double, long double, int *);
extern long double fdiml (long double, long double);
extern long double fmal (long double, long double, long double);
extern long double acoshl (long double);
extern long double atanhl (long double);
extern long double remainderl (long double, long double);
extern long double lgammal (long double);
extern long double gammal (long double);
extern long double erfl (long double);
extern long double erfcl (long double);
extern long double j0l(long double);
extern long double y0l(long double);
extern long double j1l(long double);
extern long double y1l(long double);
extern long double jnl(int, long double);
extern long double ynl(int, long double);

extern long double getpayloadl(const long double *x);

#endif /* _HAVE_LONG_DOUBLE_MATH */

#endif /* _HAVE_LONG_DOUBLE */

#endif /* __ISO_C_VISIBLE >= 1999 */

#if __MISC_VISIBLE
extern double drem (double, double);
extern float dremf (float, float);
#ifdef _HAVE_LONG_DOUBLE_MATH
extern long double dreml (long double, long double);
extern long double lgammal_r (long double, int *);
#endif
extern double lgamma_r (double, int *);
extern float lgammaf_r (float, int *);
#endif

#if __MISC_VISIBLE || __XSI_VISIBLE
extern double y0 (double);
extern double y1 (double);
extern double yn (int, double);
extern double j0 (double);
extern double j1 (double);
extern double jn (int, double);
#endif

#if __MISC_VISIBLE || __XSI_VISIBLE >= 600
extern float y0f (float);
extern float y1f (float);
extern float ynf (int, float);
extern float j0f (float);
extern float j1f (float);
extern float jnf (int, float);
#endif

/* GNU extensions */
#if __GNU_VISIBLE
extern void sincos (double, double *, double *);
extern void sincosf (float, float *, float *);
#ifdef _HAVE_LONG_DOUBLE_MATH
extern void sincosl (long double, long double *, long double *);
#endif
extern double exp10 (double);
extern double pow10 (double);
extern float exp10f (float);
extern float pow10f (float);
#ifdef _HAVE_LONG_DOUBLE_MATH
extern long double exp10l (long double);
extern long double pow10l (long double);
#endif
#endif /* __GNU_VISIBLE */

#if __MISC_VISIBLE || __XSI_VISIBLE
extern int signgam;
#endif /* __MISC_VISIBLE || __XSI_VISIBLE */

/* Useful constants.  */

#if __BSD_VISIBLE || __XSI_VISIBLE

#define MAXFLOAT	3.40282347e+38F

#define M_E		2.7182818284590452354
#define _M_E_L          2.718281828459045235360287471352662498L
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		_M_LN2
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define _M_PI_L         3.141592653589793238462643383279502884L
#define M_PI_2		1.57079632679489661923
#define _M_PI_2		1.57079632679489661923
#define _M_PI_2L	1.570796326794896619231321691639751442L
#define M_PI_4		0.78539816339744830962
#define _M_PI_4L	0.785398163397448309615660845819875721L
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440

#ifdef __GNU_VISIBLE
#define M_PIl           _M_PI_L
#define M_PI_2l         _M_PI_2L
#define M_PI_4l         _M_PI_4L
#define M_El            _M_E_L
#endif

#endif

#if __BSD_VISIBLE

#define M_TWOPI         (M_PI * 2.0)
#define M_3PI_4		2.3561944901923448370E0
#define M_SQRTPI        1.77245385090551602792981
#define M_LN2LO         1.9082149292705877000E-10
#define M_LN2HI         6.9314718036912381649E-1
#define M_SQRT3	1.73205080756887719000
#define M_IVLN10        0.43429448190325182765 /* 1 / log(10) */
#define _M_IVLN10L      0.43429448190325182765112891891660508229439700580366656611445378316586464920887L
#define M_LOG2_E        _M_LN2
#define M_INVLN2        1.4426950408889633870E0  /* 1 / log(2) */

#endif /* __BSD_VISIBLE */

#include <machine/math.h>

_END_STD_C

#ifdef __FAST_MATH__
#include <machine/fastmath.h>
#endif

#endif /* _MATH_H_ */
