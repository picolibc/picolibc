/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
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

#ifndef _MATH_H_
#define _MATH_H_

#include <sys/cdefs.h>

#define __STDC_VERSION_MATH_H__ 202311L

_BEGIN_STD_C

/*
 * Layout: macros and types, then the functions in the order of C23
 * section 7.12, split into blocks by the standard which added them,
 * then extensions. Within each section, double, float and long double
 * versions are listed in that order.
 *
 * Every long double function is declared whenever long double exists.
 * Without __HAVE_LONG_DOUBLE_MATH, picolibc only implements some of
 * them; using one of the others fails at link time.
 */

/*
 * clang 21 crashes expanding its fmaximum_num and fminimum_num
 * builtins for x87 long double (clang 23 does not). Giving the
 * declarations an asm label stops clang treating them as builtins.
 */
#if defined(__clang__) && __clang_major__ < 23 && (defined(__i386__) || defined(__x86_64__)) \
    && __LDBL_MANT_DIG__ == 64
#define __fmaxmin_numl_export(n) _ASMNAME(#n) __picolibc_export
#else
#define __fmaxmin_numl_export(n) __picolibc_export
#endif

/* Natural log of 2 */
#define _M_LN2    0.693147180559945309417
#define _M_LN2_LD 0.693147180559945309417232121458176568l

/*
 * Macros and types
 */

#ifndef HUGE_VAL
#define HUGE_VAL (__builtin_huge_val())
#endif
#if __ISO_C_VISIBLE >= 1999
#ifndef HUGE_VALF
#define HUGE_VALF (__builtin_huge_valf())
#endif
#ifndef HUGE_VALL
#define HUGE_VALL (__builtin_huge_vall())
#endif
#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif
#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif
#endif

/*
 * Specifies whether the target uses the snan/nan discriminator
 * definition from IEEE 754 2008 (top bit of significand is 1 for qNaN
 * and 0 for sNaN). This is set to zero in machine/math.h for targets
 * that don't do this (such as MIPS).
 */
#ifndef _IEEE_754_2008_SNAN
#define _IEEE_754_2008_SNAN 1
#endif

#if __ISO_C_VISIBLE >= 1999

/*
 * float_t and double_t. FLT_EVAL_METHOD 16 (from TS 18661-3) means the
 * same as 0, but also says that _Float16 exists. For other values,
 * the target is expected to provide __FLOAT_TYPE and __DOUBLE_TYPE.
 */
#if !defined(FLT_EVAL_METHOD) && defined(__FLT_EVAL_METHOD__)
#define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#define __TMP_FLT_EVAL_METHOD
#endif
#if defined FLT_EVAL_METHOD
#if (FLT_EVAL_METHOD == 0) || (FLT_EVAL_METHOD == 16)
typedef float  float_t;
typedef double double_t;
#elif FLT_EVAL_METHOD == 1
typedef double float_t;
typedef double double_t;
#elif FLT_EVAL_METHOD == 2
typedef long double float_t;
typedef long double double_t;
#elif defined(__DOUBLE_TYPE)
typedef __DOUBLE_TYPE double_t;
typedef __FLOAT_TYPE  float_t;
#endif
#else
typedef float  float_t;
typedef double double_t;
#endif
#if defined(__TMP_FLT_EVAL_METHOD)
#undef FLT_EVAL_METHOD
#endif

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#ifndef FP_ILOGB0
#define FP_ILOGB0 (-__INT_MAX__)
#endif
#ifndef FP_ILOGBNAN
#define FP_ILOGBNAN (-__INT_MAX__ - 1)
#endif

#if !defined(FP_FAST_FMA) && (__HAVE_FAST_FMA || defined(__FP_FAST_FMA))
#define FP_FAST_FMA
#endif
#if !defined(FP_FAST_FMAF) && (__HAVE_FAST_FMAF || defined(__FP_FAST_FMAF))
#define FP_FAST_FMAF
#endif
#if !defined(FP_FAST_FMAL) && (__HAVE_FAST_FMAL || defined(__FP_FAST_FMAL))
#define FP_FAST_FMAL
#endif

/* We've got a real __iscanonicall for 80-bit and double-double values */
#if __LDBL_MANT_DIG__ == 64 || (__LDBL_MANT_DIG__ == 106 && __LDBL_MAX_EXP__ == 1024)
#define __LONG_DOUBLE_NON_CANONICAL
#endif

#ifndef MATH_ERRNO
#define MATH_ERRNO 1
#endif
#ifndef MATH_ERREXCEPT
#define MATH_ERREXCEPT 2
#endif
#ifndef math_errhandling
#ifdef __IEEE_LIBM
#define _MATH_ERRHANDLING_ERRNO 0
#else
#define _MATH_ERRHANDLING_ERRNO MATH_ERRNO
#endif
#ifdef _SUPPORTS_ERREXCEPT
#define _MATH_ERRHANDLING_ERREXCEPT MATH_ERREXCEPT
#else
#define _MATH_ERRHANDLING_ERREXCEPT 0
#endif
#define math_errhandling (_MATH_ERRHANDLING_ERRNO | _MATH_ERRHANDLING_ERREXCEPT)
#endif

/* 7.12.3 Classification macros */

int __isinff(float __x) __picolibc_export;
int __isinfd(double __x) __picolibc_export;
int __isnanf(float __x) __picolibc_export;
int __isnand(double __x) __picolibc_export;
int __fpclassifyf(float __x) __picolibc_export;
int __fpclassifyd(double __x) __picolibc_export;
int __signbitf(float __x) __picolibc_export;
int __signbitd(double __x) __picolibc_export;
int __finite(double __x) __picolibc_export;
int __finitef(float __x) __picolibc_export;
int __issignalingf(float __x) __picolibc_export;
int __issignaling(double __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
int __fpclassifyl(long double __x) __picolibc_export;
int __isinfl(long double __x) __picolibc_export;
int __isnanl(long double __x) __picolibc_export;
int __finitel(long double __x) __picolibc_export;
int __signbitl(long double __x) __picolibc_export;
int __issignalingl(long double __x) __picolibc_export;
#endif

/*
 * Pick the float, double or long double version of a function by the
 * size of the argument.
 */
#ifdef __HAVE_LONG_DOUBLE
#define __float_generic3(__f, __d, __l, __x)                          \
    ((sizeof(__x) == sizeof(float))        ? __f(__x)                 \
         : (sizeof(__x) == sizeof(double)) ? __d((double)(__x))       \
                                           : __l((long double)(__x)))
#elif defined(_DOUBLE_IS_32BITS)
#define __float_generic3(__f, __d, __l, __x) __f((float)(__x))
#else
#define __float_generic3(__f, __d, __l, __x)                         \
    ((sizeof(__x) == sizeof(float)) ? __f(__x) : __d((double)(__x)))
#endif
#define __float_generic(__p, __x) __float_generic3(__p##f, __p##d, __p##l, __x)

/*
 * GCC bug 66462 raises INVALID exception when __builtin_fpclassify is
 * passed snan, so we cannot use it when building with snan support.
 * clang doesn't appear to have an option to control snan behavior, and
 * its builtin fpclassify also raises INVALID for snan, so always use
 * our version for that.
 */
#if __GNUC_PREREQ(4, 4) && (FE_INVALID != 0) && !defined(__SUPPORT_SNAN__) && !defined(__clang__)
#define fpclassify(__x)                                                                \
    (__builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, __x))
#define isfinite(__x)      (__builtin_isfinite(__x))
#define isinf(__x)         (__builtin_isinf_sign(__x))
#define isnan(__x)         (__builtin_isnan(__x))
#define isnormal(__x)      (__builtin_isnormal(__x))
#define __issubnormal(__x) (__builtin_issubnormal(__x))
#define __iszero(__x)      (__builtin_iszero(__x))
#else
#define fpclassify(__x)    __float_generic(__fpclassify, __x)
#define isfinite(__x)      __float_generic3(__finitef, __finite, __finitel, __x)
#define isinf(__x)         __float_generic(__isinf, __x)
#define isnan(__x)         __float_generic(__isnan, __x)
#define isnormal(__x)      (fpclassify(__x) == FP_NORMAL)
#define __issubnormal(__x) (fpclassify(__x) == FP_SUBNORMAL)
#define __iszero(__x)      (fpclassify(__x) == FP_ZERO)
#endif

/* C23 classification macros are C-only: C++ code may use these names as identifiers */
#if (__MISC_VISIBLE || __ISO_C_VISIBLE >= 2023) && !defined(__cplusplus)
#define issubnormal(__x) __issubnormal(__x)
#define iszero(__x)      __iszero(__x)
#ifndef issignaling
#define issignaling(__x) __float_generic3(__issignalingf, __issignaling, __issignalingl, __x)
#endif
#endif

#if __GNUC_PREREQ(4, 0)
#define signbit(__x)                                                                 \
    __float_generic3(__builtin_signbitf, __builtin_signbit, __builtin_signbitl, __x)
#else
#define signbit(__x) __float_generic(__signbit, __x)
#endif

#if (__GNU_VISIBLE || __ISO_C_VISIBLE >= 2023) && !defined(__cplusplus)
#ifdef __LONG_DOUBLE_NON_CANONICAL
int __iscanonicall(long double __x) __picolibc_export;
#define iscanonical(__x)                                                                        \
    _Generic((__x), long double: __iscanonicall(__x), default: ((void)(__typeof(__x))(__x), 1))
#else
#define iscanonical(x) ((void)(__typeof(x))(x), 1)
#endif
#endif

/* picolibc extensions */
#if __MISC_VISIBLE && !defined(__cplusplus)
#define isfinitef(x) isfinite((float)(x))
#define isinff(x)    isinf((float)(x))
#define isnanf(x)    isnan((float)(x))
#define isnormalf(x) isnormal((float)(x))
#define iszerof(x)   iszero((float)(x))
#ifdef __HAVE_LONG_DOUBLE
#define isfinitel(x) isfinite((long double)(x))
#define isinfl(x)    isinf((long double)(x))
#define isnanl(x)    isnan((long double)(x))
#define isnormall(x) isnormal((long double)(x))
#define iszerol(x)   iszero((long double)(x))
#endif
#endif /* __MISC_VISIBLE */

/* 7.12.15 Comparison macros */

#if __GNUC_PREREQ(2, 97) && !(defined(__riscv) && defined(__clang__))
#define isgreater(__x, __y)      (__builtin_isgreater(__x, __y))
#define isgreaterequal(__x, __y) (__builtin_isgreaterequal(__x, __y))
#define isless(__x, __y)         (__builtin_isless(__x, __y))
#define islessequal(__x, __y)    (__builtin_islessequal(__x, __y))
#define islessgreater(__x, __y)  (__builtin_islessgreater(__x, __y))
#define isunordered(__x, __y)    (__builtin_isunordered(__x, __y))
#else
#define isgreater(x, y)                        \
    (__extension__({                           \
        __typeof__(x) __x = (x);               \
        __typeof__(y) __y = (y);               \
        !isunordered(__x, __y) && (__x > __y); \
    }))
#define isgreaterequal(x, y)                    \
    (__extension__({                            \
        __typeof__(x) __x = (x);                \
        __typeof__(y) __y = (y);                \
        !isunordered(__x, __y) && (__x >= __y); \
    }))
#define isless(x, y)                           \
    (__extension__({                           \
        __typeof__(x) __x = (x);               \
        __typeof__(y) __y = (y);               \
        !isunordered(__x, __y) && (__x < __y); \
    }))
#define islessequal(x, y)                       \
    (__extension__({                            \
        __typeof__(x) __x = (x);                \
        __typeof__(y) __y = (y);                \
        !isunordered(__x, __y) && (__x <= __y); \
    }))
#define islessgreater(x, y)                                 \
    (__extension__({                                        \
        __typeof__(x) __x = (x);                            \
        __typeof__(y) __y = (y);                            \
        !isunordered(__x, __y) && (__x < __y || __x > __y); \
    }))
#define isunordered(a, b)                                       \
    (__extension__({                                            \
        __typeof__(a) __a = (a);                                \
        __typeof__(b) __b = (b);                                \
        fpclassify(__a) == FP_NAN || fpclassify(__b) == FP_NAN; \
    }))
#endif

int __iseqsig(double __x, double __y) __picolibc_export;
int __iseqsigf(float __x, float __y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
int __iseqsigl(long double __x, long double __y) __picolibc_export;
#endif

#if (__MISC_VISIBLE || __ISO_C_VISIBLE >= 2023) && !defined(__cplusplus)
#ifdef __HAVE_LONG_DOUBLE
#define iseqsig(__x, __y)                                                \
    ((sizeof((__x) + (__y)) == sizeof(float)) ? __iseqsigf((__x), (__y)) \
         : (sizeof((__x) + (__y)) == sizeof(double))                     \
         ? __iseqsig((double)(__x), (double)(__y))                       \
         : __iseqsigl((long double)(__x), (long double)(__y)))
#else
#define iseqsig(__x, __y)                                                                \
    ((sizeof((__x) + (__y)) == sizeof(float)) ? __iseqsigf((__x), (__y))                 \
                                              : __iseqsig((double)(__x), (double)(__y)))
#endif
#endif

#endif /* __ISO_C_VISIBLE >= 1999 */

#if __GNU_VISIBLE || __ISO_C_VISIBLE >= 2023

/* Rounding modes for fromfp functions */
#define FP_INT_UPWARD            0
#define FP_INT_DOWNWARD          1
#define FP_INT_TOWARDZERO        2
#define FP_INT_TONEARESTFROMZERO 3
#define FP_INT_TONEAREST         4

/* Return values from llogb */
#define FP_LLOGB0   (-__LONG_MAX__)
#define FP_LLOGBNAN (-__LONG_MAX__ - 1)

#endif /* __GNU_VISIBLE || __ISO_C_VISIBLE >= 2023 */

/*
 * C89 functions. The C89 standard reserves the float and long double
 * names, so they are declared here as well.
 */

/* 7.12.4 Trigonometric functions */
double acos(double __x) __picolibc_export;
double asin(double __x) __picolibc_export;
double atan(double __x) __picolibc_export;
double atan2(double __y, double __x) __picolibc_export;
double cos(double __x) __picolibc_export;
double sin(double __x) __picolibc_export;
double tan(double __x) __picolibc_export;
float  acosf(float __x) __picolibc_export;
float  asinf(float __x) __picolibc_export;
float  atanf(float __x) __picolibc_export;
float  atan2f(float __y, float __x) __picolibc_export;
float  cosf(float __x) __picolibc_export;
float  sinf(float __x) __picolibc_export;
float  tanf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double acosl(long double __x) __picolibc_export;
long double asinl(long double __x) __picolibc_export;
long double atanl(long double __x) __picolibc_export;
long double atan2l(long double __y, long double __x) __picolibc_export;
long double cosl(long double __x) __picolibc_export;
long double sinl(long double __x) __picolibc_export;
long double tanl(long double __x) __picolibc_export;
#endif

/* 7.12.5 Hyperbolic functions */
double cosh(double __x) __picolibc_export;
double sinh(double __x) __picolibc_export;
double tanh(double __x) __picolibc_export;
float  coshf(float __x) __picolibc_export;
float  sinhf(float __x) __picolibc_export;
float  tanhf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double coshl(long double __x) __picolibc_export;
long double sinhl(long double __x) __picolibc_export;
long double tanhl(long double __x) __picolibc_export;
#endif

/* 7.12.6 Exponential and logarithmic functions */
double exp(double __x) __picolibc_export;
double frexp(double __value, int *__exp) __picolibc_export;
double ldexp(double __x, int __exp) __picolibc_export;
double log(double __x) __picolibc_export;
double log10(double __x) __picolibc_export;
double modf(double __value, double *__iptr) __picolibc_export;
float  expf(float __x) __picolibc_export;
float  frexpf(float __value, int *__exp) __picolibc_export;
float  ldexpf(float __x, int __exp) __picolibc_export;
float  logf(float __x) __picolibc_export;
float  log10f(float __x) __picolibc_export;
float  modff(float __value, float *__iptr) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double expl(long double __x) __picolibc_export;
long double frexpl(long double __value, int *__exp) __picolibc_export;
long double ldexpl(long double __x, int __exp) __picolibc_export;
long double logl(long double __x) __picolibc_export;
long double log10l(long double __x) __picolibc_export;
long double modfl(long double __value, long double *__iptr) __picolibc_export;
#endif

/* 7.12.7 Power and absolute-value functions */
double fabs(double __x) __picolibc_export;
double pow(double __x, double __y) __picolibc_export;
double sqrt(double __x) __picolibc_export;
float  fabsf(float __x) __picolibc_export;
float  powf(float __x, float __y) __picolibc_export;
float  sqrtf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double fabsl(long double __x) __picolibc_export;
long double powl(long double __x, long double __y) __picolibc_export;
long double sqrtl(long double __x) __picolibc_export;
#endif

/* 7.12.9 Nearest integer functions */
double ceil(double __x) __picolibc_export;
double floor(double __x) __picolibc_export;
float  ceilf(float __x) __picolibc_export;
float  floorf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double ceill(long double __x) __picolibc_export;
long double floorl(long double __x) __picolibc_export;
#endif

/* 7.12.10 Remainder functions */
double fmod(double __x, double __y) __picolibc_export;
float  fmodf(float __x, float __y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double fmodl(long double __x, long double __y) __picolibc_export;
#endif

/*
 * C99 functions
 */
#if __ISO_C_VISIBLE >= 1999

/* 7.12.5 Hyperbolic functions */
double acosh(double __x) __picolibc_export;
double asinh(double __x) __picolibc_export;
double atanh(double __x) __picolibc_export;
float  acoshf(float __x) __picolibc_export;
float  asinhf(float __x) __picolibc_export;
float  atanhf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double acoshl(long double __x) __picolibc_export;
long double asinhl(long double __x) __picolibc_export;
long double atanhl(long double __x) __picolibc_export;
#endif

/* 7.12.6 Exponential and logarithmic functions */
double exp2(double __x) __picolibc_export;
double expm1(double __x) __picolibc_export;
int    ilogb(double __x) __picolibc_export;
double log1p(double __x) __picolibc_export;
double log2(double __x) __picolibc_export;
double logb(double __x) __picolibc_export;
double scalbn(double __x, int __n) __picolibc_export;
double scalbln(double __x, long int __n) __picolibc_export;
float  exp2f(float __x) __picolibc_export;
float  expm1f(float __x) __picolibc_export;
int    ilogbf(float __x) __picolibc_export;
float  log1pf(float __x) __picolibc_export;
float  log2f(float __x) __picolibc_export;
float  logbf(float __x) __picolibc_export;
float  scalbnf(float __x, int __n) __picolibc_export;
float  scalblnf(float __x, long int __n) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double exp2l(long double __x) __picolibc_export;
long double expm1l(long double __x) __picolibc_export;
int         ilogbl(long double __x) __picolibc_export;
long double log1pl(long double __x) __picolibc_export;
long double log2l(long double __x) __picolibc_export;
long double logbl(long double __x) __picolibc_export;
long double scalbnl(long double __x, int __n) __picolibc_export;
long double scalblnl(long double __x, long int __n) __picolibc_export;
#endif

/* 7.12.7 Power and absolute-value functions */
double cbrt(double __x) __picolibc_export;
double hypot(double __x, double __y) __picolibc_export;
float  cbrtf(float __x) __picolibc_export;
float  hypotf(float __x, float __y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double cbrtl(long double __x) __picolibc_export;
long double hypotl(long double __x, long double __y) __picolibc_export;
#endif

/* 7.12.8 Error and gamma functions */
double erf(double __x) __picolibc_export;
double erfc(double __x) __picolibc_export;
double lgamma(double __x) __picolibc_export;
double tgamma(double __x) __picolibc_export;
float  erff(float __x) __picolibc_export;
float  erfcf(float __x) __picolibc_export;
float  lgammaf(float __x) __picolibc_export;
float  tgammaf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double erfl(long double __x) __picolibc_export;
long double erfcl(long double __x) __picolibc_export;
long double lgammal(long double __x) __picolibc_export;
long double tgammal(long double __x) __picolibc_export;
#endif

/* 7.12.9 Nearest integer functions */
double        nearbyint(double __x) __picolibc_export;
double        rint(double __x) __picolibc_export;
long int      lrint(double __x) __picolibc_export;
long long int llrint(double __x) __picolibc_export;
double        round(double __x) __picolibc_export;
long int      lround(double __x) __picolibc_export;
long long int llround(double __x) __picolibc_export;
double        trunc(double __x) __picolibc_export;
float         nearbyintf(float __x) __picolibc_export;
float         rintf(float __x) __picolibc_export;
long int      lrintf(float __x) __picolibc_export;
long long int llrintf(float __x) __picolibc_export;
float         roundf(float __x) __picolibc_export;
long int      lroundf(float __x) __picolibc_export;
long long int llroundf(float __x) __picolibc_export;
float         truncf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double   nearbyintl(long double __x) __picolibc_export;
long double   rintl(long double __x) __picolibc_export;
long int      lrintl(long double __x) __picolibc_export;
long long int llrintl(long double __x) __picolibc_export;
long double   roundl(long double __x) __picolibc_export;
long int      lroundl(long double __x) __picolibc_export;
long long int llroundl(long double __x) __picolibc_export;
long double   truncl(long double __x) __picolibc_export;
#endif

/* 7.12.10 Remainder functions */
double remainder(double __x, double __y) __picolibc_export;
double remquo(double __x, double __y, int *__quo) __picolibc_export;
float  remainderf(float __x, float __y) __picolibc_export;
float  remquof(float __x, float __y, int *__quo) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double remainderl(long double __x, long double __y) __picolibc_export;
long double remquol(long double __x, long double __y, int *__quo) __picolibc_export;
#endif

/* 7.12.11 Manipulation functions */
double copysign(double __x, double __y) __picolibc_export;
double nan(const char *__tagp) __picolibc_export;
double nextafter(double __x, double __y) __picolibc_export;
float  copysignf(float __x, float __y) __picolibc_export;
float  nanf(const char *__tagp) __picolibc_export;
float  nextafterf(float __x, float __y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
double      nexttoward(double __x, long double __y) __picolibc_export;
float       nexttowardf(float __x, long double __y) __picolibc_export;
long double copysignl(long double __x, long double __y) __picolibc_export;
long double nanl(const char *__tagp) __picolibc_export;
long double nextafterl(long double __x, long double __y) __picolibc_export;
long double nexttowardl(long double __x, long double __y) __picolibc_export;
#endif

/* 7.12.12 Maximum, minimum, and positive difference functions */
double fdim(double __x, double __y) __picolibc_export;
double fmax(double __x, double __y) __picolibc_export;
double fmin(double __x, double __y) __picolibc_export;
float  fdimf(float __x, float __y) __picolibc_export;
float  fmaxf(float __x, float __y) __picolibc_export;
float  fminf(float __x, float __y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double fdiml(long double __x, long double __y) __picolibc_export;
long double fmaxl(long double __x, long double __y) __picolibc_export;
long double fminl(long double __x, long double __y) __picolibc_export;
#endif

/* 7.12.13 Floating multiply-add */
double fma(double __x, double __y, double __z) __picolibc_export;
float  fmaf(float __x, float __y, float __z) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double fmal(long double __x, long double __y, long double __z) __picolibc_export;
#endif

#endif /* __ISO_C_VISIBLE >= 1999 */

/*
 * C23 functions
 */
#if __GNU_VISIBLE || __ISO_C_VISIBLE >= 2023

/* 7.12.4 Trigonometric functions */
double acospi(double __x) __picolibc_export;
double asinpi(double __x) __picolibc_export;
double atanpi(double __x) __picolibc_export;
double atan2pi(double __y, double __x) __picolibc_export;
double cospi(double __x) __picolibc_export;
double sinpi(double __x) __picolibc_export;
double tanpi(double __x) __picolibc_export;
float  acospif(float __x) __picolibc_export;
float  asinpif(float __x) __picolibc_export;
float  atanpif(float __x) __picolibc_export;
float  atan2pif(float __y, float __x) __picolibc_export;
float  cospif(float __x) __picolibc_export;
float  sinpif(float __x) __picolibc_export;
float  tanpif(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double acospil(long double __x) __picolibc_export;
long double asinpil(long double __x) __picolibc_export;
long double atanpil(long double __x) __picolibc_export;
long double atan2pil(long double __y, long double __x) __picolibc_export;
long double cospil(long double __x) __picolibc_export;
long double sinpil(long double __x) __picolibc_export;
long double tanpil(long double __x) __picolibc_export;
#endif

/* 7.12.6 Exponential and logarithmic functions */
double   exp10(double __x) __picolibc_export;
double   exp10m1(double __x) __picolibc_export;
double   exp2m1(double __x) __picolibc_export;
long int llogb(double __x) __picolibc_export;
double   log10p1(double __x) __picolibc_export;
double   log2p1(double __x) __picolibc_export;
double   logp1(double __x) __picolibc_export;
float    exp10f(float __x) __picolibc_export;
float    exp10m1f(float __x) __picolibc_export;
float    exp2m1f(float __x) __picolibc_export;
long int llogbf(float __x) __picolibc_export;
float    log10p1f(float __x) __picolibc_export;
float    log2p1f(float __x) __picolibc_export;
float    logp1f(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double exp10l(long double __x) __picolibc_export;
long double exp10m1l(long double __x) __picolibc_export;
long double exp2m1l(long double __x) __picolibc_export;
long int    llogbl(long double __x) __picolibc_export;
long double log10p1l(long double __x) __picolibc_export;
long double log2p1l(long double __x) __picolibc_export;
long double logp1l(long double __x) __picolibc_export;
#endif

/* 7.12.7 Power and absolute-value functions */
double compoundn(double __x, long long int __n) __picolibc_export;
double pown(double __x, long long int __n) __picolibc_export;
double powr(double __x, double __y) __picolibc_export;
double rootn(double __x, long long int __n) __picolibc_export;
double rsqrt(double __x) __picolibc_export;
float  compoundnf(float __x, long long int __n) __picolibc_export;
float  pownf(float __x, long long int __n) __picolibc_export;
float  powrf(float __x, float __y) __picolibc_export;
float  rootnf(float __x, long long int __n) __picolibc_export;
float  rsqrtf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double compoundnl(long double __x, long long int __n) __picolibc_export;
long double pownl(long double __x, long long int __n) __picolibc_export;
long double powrl(long double __x, long double __y) __picolibc_export;
long double rootnl(long double __x, long long int __n) __picolibc_export;
long double rsqrtl(long double __x) __picolibc_export;
#endif

/* 7.12.9 Nearest integer functions */
double roundeven(double __x) __picolibc_export;
double fromfp(double __x, int __rnd, unsigned int __width) __picolibc_export;
double ufromfp(double __x, int __rnd, unsigned int __width) __picolibc_export;
double fromfpx(double __x, int __rnd, unsigned int __width) __picolibc_export;
double ufromfpx(double __x, int __rnd, unsigned int __width) __picolibc_export;
float  roundevenf(float __x) __picolibc_export;
float  fromfpf(float __x, int __rnd, unsigned int __width) __picolibc_export;
float  ufromfpf(float __x, int __rnd, unsigned int __width) __picolibc_export;
float  fromfpxf(float __x, int __rnd, unsigned int __width) __picolibc_export;
float  ufromfpxf(float __x, int __rnd, unsigned int __width) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double roundevenl(long double __x) __picolibc_export;
long double fromfpl(long double __x, int __rnd, unsigned int __width) __picolibc_export;
long double ufromfpl(long double __x, int __rnd, unsigned int __width) __picolibc_export;
long double fromfpxl(long double __x, int __rnd, unsigned int __width) __picolibc_export;
long double ufromfpxl(long double __x, int __rnd, unsigned int __width) __picolibc_export;
#endif

/* 7.12.11 Manipulation functions */
int    canonicalize(double *__cx, const double *__x) __picolibc_export;
double nextdown(double __x) __picolibc_export;
double nextup(double __x) __picolibc_export;
int    canonicalizef(float *__cx, const float *__x) __picolibc_export;
float  nextdownf(float __x) __picolibc_export;
float  nextupf(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
int         canonicalizel(long double *__cx, const long double *__x) __picolibc_export;
long double nextdownl(long double __x) __picolibc_export;
long double nextupl(long double __x) __picolibc_export;
#endif

/* 7.12.12 Maximum, minimum, and positive difference functions */
double fmaximum(double __x, double __y) __picolibc_export;
double fminimum(double __x, double __y) __picolibc_export;
double fmaximum_mag(double __x, double __y) __picolibc_export;
double fminimum_mag(double __x, double __y) __picolibc_export;
double fmaximum_num(double __x, double __y) __picolibc_export;
double fminimum_num(double __x, double __y) __picolibc_export;
double fmaximum_mag_num(double __x, double __y) __picolibc_export;
double fminimum_mag_num(double __x, double __y) __picolibc_export;
float  fmaximumf(float __x, float __y) __picolibc_export;
float  fminimumf(float __x, float __y) __picolibc_export;
float  fmaximum_magf(float __x, float __y) __picolibc_export;
float  fminimum_magf(float __x, float __y) __picolibc_export;
float  fmaximum_numf(float __x, float __y) __picolibc_export;
float  fminimum_numf(float __x, float __y) __picolibc_export;
float  fmaximum_mag_numf(float __x, float __y) __picolibc_export;
float  fminimum_mag_numf(float __x, float __y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double fmaximuml(long double __x, long double __y) __picolibc_export;
long double fminimuml(long double __x, long double __y) __picolibc_export;
long double fmaximum_magl(long double __x, long double __y) __picolibc_export;
long double fminimum_magl(long double __x, long double __y) __picolibc_export;
long double fmaximum_numl(long double __x, long double __y) __fmaxmin_numl_export(fmaximum_numl);
long double fminimum_numl(long double __x, long double __y) __fmaxmin_numl_export(fminimum_numl);
long double fmaximum_mag_numl(long double __x, long double __y) __picolibc_export;
long double fminimum_mag_numl(long double __x, long double __y) __picolibc_export;
#endif

/* 7.12.14 Functions that round result to narrower type */
float fadd(double __x, double __y) __picolibc_export;
float fsub(double __x, double __y) __picolibc_export;
float fmul(double __x, double __y) __picolibc_export;
float fdiv(double __x, double __y) __picolibc_export;
float ffma(double __x, double __y, double __z) __picolibc_export;
float fsqrt(double __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
float  faddl(long double __x, long double __y) __picolibc_export;
float  fsubl(long double __x, long double __y) __picolibc_export;
float  fmull(long double __x, long double __y) __picolibc_export;
float  fdivl(long double __x, long double __y) __picolibc_export;
float  ffmal(long double __x, long double __y, long double __z) __picolibc_export;
float  fsqrtl(long double __x) __picolibc_export;
double daddl(long double __x, long double __y) __picolibc_export;
double dsubl(long double __x, long double __y) __picolibc_export;
double dmull(long double __x, long double __y) __picolibc_export;
double ddivl(long double __x, long double __y) __picolibc_export;
double dfmal(long double __x, long double __y, long double __z) __picolibc_export;
double dsqrtl(long double __x) __picolibc_export;
#endif

/* F.10.12 Total order functions */
int totalorder(const double *__x, const double *__y) __picolibc_export;
int totalordermag(const double *__x, const double *__y) __picolibc_export;
int totalorderf(const float *__x, const float *__y) __picolibc_export;
int totalordermagf(const float *__x, const float *__y) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
int totalorderl(const long double *__x, const long double *__y) __picolibc_export;
int totalordermagl(const long double *__x, const long double *__y) __picolibc_export;
#endif

/* F.10.13 Payload functions */
double getpayload(const double *__x) __picolibc_export;
int    setpayload(double *__res, double __pl) __picolibc_export;
int    setpayloadsig(double *__res, double __pl) __picolibc_export;
float  getpayloadf(const float *__x) __picolibc_export;
int    setpayloadf(float *__res, float __pl) __picolibc_export;
int    setpayloadsigf(float *__res, float __pl) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double getpayloadl(const long double *__x) __picolibc_export;
int         setpayloadl(long double *__res, long double __pl) __picolibc_export;
int         setpayloadsigl(long double *__res, long double __pl) __picolibc_export;
#endif

#endif /* __GNU_VISIBLE || __ISO_C_VISIBLE >= 2023 */

/*
 * Extensions
 */

/* GNU */
#if __GNU_VISIBLE
void   sincos(double __x, double *__sinx, double *__cosx) __picolibc_export;
double pow10(double __x) __picolibc_export;
void   sincosf(float __x, float *__sinx, float *__cosx) __picolibc_export;
float  pow10f(float __x) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
void        sincosl(long double __x, long double *__sinx, long double *__cosx) __picolibc_export;
long double pow10l(long double __x) __picolibc_export;
#endif
#endif /* __GNU_VISIBLE */

#if __MISC_VISIBLE || __XSI_VISIBLE
extern __picolibc_export int signgam;
#endif

/* Bessel functions. XSI only specifies the double versions. */
#if __MISC_VISIBLE || __XSI_VISIBLE
double j0(double __x) __picolibc_export;
double j1(double __x) __picolibc_export;
double jn(int __n, double __x) __picolibc_export;
double y0(double __x) __picolibc_export;
double y1(double __x) __picolibc_export;
double yn(int __n, double __x) __picolibc_export;
#endif
#if __MISC_VISIBLE || __XSI_VISIBLE >= 600
float j0f(float __x) __picolibc_export;
float j1f(float __x) __picolibc_export;
float jnf(int __n, float __x) __picolibc_export;
float y0f(float __x) __picolibc_export;
float y1f(float __x) __picolibc_export;
float ynf(int __n, float __x) __picolibc_export;
#endif
#if __MISC_VISIBLE && defined(__HAVE_LONG_DOUBLE)
long double j0l(long double __x) __picolibc_export;
long double j1l(long double __x) __picolibc_export;
long double jnl(int __n, long double __x) __picolibc_export;
long double y0l(long double __x) __picolibc_export;
long double y1l(long double __x) __picolibc_export;
long double ynl(int __n, long double __x) __picolibc_export;
#endif

/*
 * Legacy BSD and SVID functions. The classification functions share
 * their names with macros, so the names are in parentheses.
 */
#if __MISC_VISIBLE
int finite(double __x) __picolibc_export;
int finitef(float __x) __picolibc_export;
int(isinff)(float __x) __picolibc_export;
int(isnanf)(float __x) __picolibc_export;
#if !defined(__cplusplus) || __cplusplus < 201103L
int(isinf)(double __x) __picolibc_export;
#endif

double drem(double __x, double __y) __picolibc_export;
double gamma(double __x) __picolibc_export;
double infinity(void) __picolibc_export;
double lgamma_r(double __x, int *__signgamp) __picolibc_export;
double significand(double __x) __picolibc_export;
float  dremf(float __x, float __y) __picolibc_export;
float  gammaf(float __x) __picolibc_export;
float  infinityf(void) __picolibc_export;
float  lgammaf_r(float __x, int *__signgamp) __picolibc_export;
float  significandf(float __x) __picolibc_export;

#ifdef __HAVE_LONG_DOUBLE
int finitel(long double __x) __picolibc_export;
int(isinfl)(long double __x) __picolibc_export;
int(isnanl)(long double __x) __picolibc_export;
long double dreml(long double __x, long double __y) __picolibc_export;
long double gammal(long double __x) __picolibc_export;
long double infinityl(void) __picolibc_export;
long double lgammal_r(long double __x, int *__signgamp) __picolibc_export;
long double significandl(long double __x) __picolibc_export;
#endif
#endif /* __MISC_VISIBLE */

#if __MISC_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 700)
double scalb(double __x, double __n) __picolibc_export;
float  scalbf(float __x, float __n) __picolibc_export;
#ifdef __HAVE_LONG_DOUBLE
long double scalbl(long double __x, long double __n) __picolibc_export;
#endif
#endif

#if (__MISC_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 600)) \
    && (!defined(__cplusplus) || __cplusplus < 201103L)
int(isnan)(double __x) __picolibc_export;
#endif

/* Useful constants.  */

#if __BSD_VISIBLE || __XSI_VISIBLE

#define MAXFLOAT   3.40282347e+38F

#define M_E        2.7182818284590452354
#define _M_E_L     2.718281828459045235360287471352662498L
#define M_LOG2E    1.4426950408889634074
#define M_LOG10E   0.43429448190325182765
#define M_LN2      _M_LN2
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define _M_PI_L    3.141592653589793238462643383279502884L
#define M_PI_2     1.57079632679489661923
#define _M_PI_2    1.57079632679489661923
#define _M_PI_2L   1.570796326794896619231321691639751442L
#define M_PI_4     0.78539816339744830962
#define _M_PI_4L   0.785398163397448309615660845819875721L
#define M_1_PI     0.31830988618379067154
#define M_2_PI     0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.70710678118654752440

#if __GNU_VISIBLE
#define M_PIl   _M_PI_L
#define M_PI_2l _M_PI_2L
#define M_PI_4l _M_PI_4L
#define M_El    _M_E_L
#endif

#endif

#if __BSD_VISIBLE

#define M_TWOPI    (M_PI * 2.0)
#define M_3PI_4    2.3561944901923448370E0
#define M_SQRTPI   1.77245385090551602792981
#define M_LN2LO    1.9082149292705877000E-10
#define M_LN2HI    6.9314718036912381649E-1
#define M_SQRT3    1.73205080756887719000
#define M_IVLN10   0.43429448190325182765 /* 1 / log(10) */
#define _M_IVLN10L 0.43429448190325182765112891891660508229439700580366656611445378316586464920887L
#define M_LOG2_E   _M_LN2
#define M_INVLN2   1.4426950408889633870E0 /* 1 / log(2) */

#endif /* __BSD_VISIBLE */

#include <machine/math.h>

#ifdef __FAST_MATH__
#include <machine/fastmath.h>
#endif

_END_STD_C

#endif /* _MATH_H_ */
