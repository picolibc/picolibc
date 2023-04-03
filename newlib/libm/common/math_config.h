/* Configuration for math routines.
   Copyright (c) 2017-2018 Arm Ltd.  All rights reserved.

   SPDX-License-Identifier: BSD-3-Clause

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the company may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef _MATH_CONFIG_H
#define _MATH_CONFIG_H

#include <math.h>
#include <stdint.h>
#include <errno.h>
#include <fenv.h>

#ifndef WANT_ROUNDING
/* Correct special case results in non-nearest rounding modes.  */
# define WANT_ROUNDING 1
#endif
#ifdef _IEEE_LIBM
# define WANT_ERRNO 0
# define _LIB_VERSION _IEEE_
#else
/* Set errno according to ISO C with (math_errhandling & MATH_ERRNO) != 0.  */
# define WANT_ERRNO 1
# define _LIB_VERSION _POSIX_
#endif
#ifndef WANT_ERRNO_UFLOW
/* Set errno to ERANGE if result underflows to 0 (in all rounding modes).  */
# define WANT_ERRNO_UFLOW (WANT_ROUNDING && WANT_ERRNO)
#endif

#define _IEEE_  -1
#define _POSIX_ 0

#ifdef _HAVE_ATTRIBUTE_ALWAYS_INLINE
#define ALWAYS_INLINE __inline__ __attribute__((__always_inline__))
#else
#define ALWAYS_INLINE __inline__
#endif

#ifdef _HAVE_ATTRIBUTE_NOINLINE
# define NOINLINE __attribute__ ((__noinline__))
#else
# define NOINLINE
#endif

#ifdef _HAVE_BUILTIN_EXPECT
# define likely(x) __builtin_expect (!!(x), 1)
# define unlikely(x) __builtin_expect (x, 0)
#else
# define likely(x) (x)
# define unlikely(x) (x)
#endif

/* Compiler can inline round as a single instruction.  */
#ifndef HAVE_FAST_ROUND
# if __aarch64__
#   define HAVE_FAST_ROUND 1
# else
#   define HAVE_FAST_ROUND 0
# endif
#endif

/* Compiler can inline lround, but not (long)round(x).  */
#ifndef HAVE_FAST_LROUND
# if __aarch64__ && (100*__GNUC__ + __GNUC_MINOR__) >= 408 && __NO_MATH_ERRNO__
#   define HAVE_FAST_LROUND 1
# else
#   define HAVE_FAST_LROUND 0
# endif
#endif

#if HAVE_FAST_ROUND
/* When set, the roundtoint and converttoint functions are provided with
   the semantics documented below.  */
# define TOINT_INTRINSICS 1

/* Round x to nearest int in all rounding modes, ties have to be rounded
   consistently with converttoint so the results match.  If the result
   would be outside of [-2^31, 2^31-1] then the semantics is unspecified.  */
static ALWAYS_INLINE double_t
roundtoint (double_t x)
{
  return round (x);
}

/* Convert x to nearest int in all rounding modes, ties have to be rounded
   consistently with roundtoint.  If the result is not representible in an
   int32_t then the semantics is unspecified.  */
static ALWAYS_INLINE int32_t
converttoint (double_t x)
{
# if HAVE_FAST_LROUND
  return lround (x);
# else
  return (long) round (x);
# endif
}
#endif

#ifndef TOINT_INTRINSICS
# define TOINT_INTRINSICS 0
#endif

static ALWAYS_INLINE uint32_t
asuint (float f)
{
#if defined(__riscv_flen) && __riscv_flen >= 32
  uint32_t result;
  __asm__("fmv.x.w\t%0, %1" : "=r" (result) : "f" (f));
  return result;
#else
  union
  {
    float f;
    uint32_t i;
  } u = {f};
  return u.i;
#endif
}

static ALWAYS_INLINE float
asfloat (uint32_t i)
{
#if defined(__riscv_flen) && __riscv_flen >= 32
  float result;
  __asm__("fmv.w.x\t%0, %1" : "=f" (result) : "r" (i));
  return result;
#else
  union
  {
    uint32_t i;
    float f;
  } u = {i};
  return u.f;
#endif
}

static ALWAYS_INLINE int32_t
_asint32 (float f)
{
    return (int32_t) asuint(f);
}

static ALWAYS_INLINE int
_sign32(int32_t ix)
{
    return ((uint32_t) ix) >> 31;
}

static ALWAYS_INLINE int
_exponent32(int32_t ix)
{
    return (ix >> 23) & 0xff;
}

static ALWAYS_INLINE int32_t
_significand32(int32_t ix)
{
    return ix & 0x7fffff;
}

static ALWAYS_INLINE float
_asfloat(int32_t i)
{
    return asfloat((uint32_t) i);
}

static ALWAYS_INLINE uint64_t
asuint64 (double f)
{
#if defined(__riscv_flen) && __riscv_flen >= 64 && __riscv_xlen >= 64
  uint64_t result;
  __asm__("fmv.x.d\t%0, %1" : "=r" (result) : "f" (f));
  return result;
#else
  union
  {
    double f;
    uint64_t i;
  } u = {f};
  return u.i;
#endif
}

static ALWAYS_INLINE double
asdouble (uint64_t i)
{
#if defined(__riscv_flen) && __riscv_flen >= 64 && __riscv_xlen >= 64
  double result;
  __asm__("fmv.d.x\t%0, %1" : "=f" (result) : "r" (i));
  return result;
#else
  union
  {
    uint64_t i;
    double f;
  } u = {i};
  return u.f;
#endif
}

static ALWAYS_INLINE int64_t
_asint64(double f)
{
    return (int64_t) asuint64(f);
}

static ALWAYS_INLINE int
_sign64(int64_t ix)
{
    return ((uint64_t) ix) >> 63;
}

static ALWAYS_INLINE int
_exponent64(int64_t ix)
{
    return (ix >> 52) & 0x7ff;
}

static ALWAYS_INLINE int64_t
_significand64(int64_t ix)
{
    return ix & 0xfffffffffffffLL;
}

static ALWAYS_INLINE double
_asdouble(int64_t i)
{
    return asdouble((uint64_t) i);
}

#ifndef IEEE_754_2008_SNAN
# define IEEE_754_2008_SNAN 1
#endif
static ALWAYS_INLINE int
issignalingf_inline (float x)
{
  uint32_t ix = asuint (x);
  if (!IEEE_754_2008_SNAN)
    return (ix & 0x7fc00000) == 0x7fc00000;
  return 2 * (ix ^ 0x00400000) > 0xFF800000u;
}

static ALWAYS_INLINE int
issignaling_inline (double x)
{
  uint64_t ix = asuint64 (x);
  if (!IEEE_754_2008_SNAN)
    return (ix & 0x7ff8000000000000) == 0x7ff8000000000000;
  return 2 * (ix ^ 0x0008000000000000) > 2 * 0x7ff8000000000000ULL;
}

#ifdef PICOLIBC_FLOAT_NOEXCEPT
#define FORCE_FLOAT     float
#define pick_float_except(expr,val)    (val)
#else
#define FORCE_FLOAT     volatile float
#define pick_float_except(expr,val)    (expr)
#endif

#ifdef PICOLIBC_DOUBLE_NOEXCEPT
#define FORCE_DOUBLE    double
#define pick_double_except(expr,val)    (val)
#else
#define FORCE_DOUBLE    volatile double
#define pick_double_except(expr,val)    (expr)
#endif

#ifdef PICOLIBC_LONG_DOUBLE_NOEXCEPT
#define FORCE_LONG_DOUBLE       long double
#define pick_long_double_except(expr,val)       (val)
#else
#define FORCE_LONG_DOUBLE       volatile long double
#define pick_long_double_except(expr,val)       (expr)
#endif

static ALWAYS_INLINE float
opt_barrier_float (float x)
{
  FORCE_FLOAT y = x;
  return y;
}

static ALWAYS_INLINE double
opt_barrier_double (double x)
{
  FORCE_DOUBLE y = x;
  return y;
}

static ALWAYS_INLINE void
force_eval_float (float x)
{
  FORCE_FLOAT y = x;
  (void) y;
}

static ALWAYS_INLINE void
force_eval_double (double x)
{
  FORCE_DOUBLE y = x;
  (void) y;
}

#ifdef _HAVE_LONG_DOUBLE
static ALWAYS_INLINE void
force_eval_long_double (long double x)
{
    FORCE_LONG_DOUBLE y = x;
    (void) y;
}
#endif

/* Clang doesn't appear to suppor precise exceptions on
 * many targets. We introduce barriers for that compiler
 * to force evaluation order where needed
 */
#ifdef __clang__
#define clang_barrier_double(x) opt_barrier_double(x)
#define clang_barrier_float(x) opt_barrier_float(x)
#define clang_force_double(x) force_eval_double(x)
#define clang_force_float(x) force_eval_float(x)
#else
#define clang_barrier_double(x) (x)
#define clang_barrier_float(x) (x)
#define clang_force_double(x) (x)
#define clang_force_float(x) (x)
#endif

#ifdef _ADD_UNDER_R_TO_FUNCS

#define __FLOAT_NAME(x) x ## f_r
#define _FLOAT_NAME(x) __FLOAT_NAME(x)
#define _FLOAT_ALIAS(x) # x "f_r"

#define __LD_NAME(x) x ## l_r
#define _LD_NAME(x) __LD_NAME(x)
#define _LD_ALIAS(x) # x "l_r"

#else

#define __FLOAT_NAME(x) x ## f
#define _FLOAT_NAME(x) __FLOAT_NAME(x)
#define _FLOAT_ALIAS(x) # x "f"

#define __LD_NAME(x) x ## l
#define _LD_NAME(x) __LD_NAME(x)
#define _LD_ALIAS(x) # x "l"

#endif

#define __FLOAT_NAME_REG(x) x ## f
#define _FLOAT_NAME_REG(x) __FLOAT_NAME_REG(x)

#define __LD_NAME_REG(x) x ## l
#define _LD_NAME_REG(x) __LD_NAME_REG(x)

#ifdef _ADD_D_TO_DOUBLE_FUNCS

#define __D_NAME(x) x ## d
#define _D_NAME(x) __D_NAME(x)
#define _D_ALIAS(x) # x "d"

#elif defined(_ADD_UNDER_R_TO_FUNCS)

#define __D_NAME(x) x ## _r
#define _D_NAME(x) __D_NAME(x)
#define _D_ALIAS(x) # x "_r"

#else

#define __D_NAME(x) x
#define _D_NAME(x) __D_NAME(x)
#define _D_ALIAS(x) # x

#endif

#define __D_NAME_REG(x) x
#define _D_NAME_REG(x) __D_NAME_REG(x)

/*
 * Figure out how to map the 32- and 64- bit functions to
 * the three C float types (float, double, long double).
 *
 * 'float' is assumed to be a 32-bit IEEE value.
 *
 * Internal names for 64-bit funcs are unqualified,
 * Internal names for 32-bit funcs have a trailing 'f'
 * Internal names for longer funcs have a trailing 'l'
 *
 * When types are the same size, they are assumed to have the same
 * representation. Aliases are generated in this case to eliminate
 * overhead.
 *
 * 32-bit functions are always 'float'
 * 64-bit functions may either be 'double' or 'long double',
 * if at least one of those is 64 bits in size
 *
 * There is limited support for long double greater than 64 bits
 */

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wattribute-alias="
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif

#ifdef _DOUBLE_IS_32BITS
# ifdef _HAVE_ALIAS_ATTRIBUTE
#  define _MATH_ALIAS_d_to_f(name) extern double _D_NAME(name)(void) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_d_to_f(name) extern double _D_NAME(name)(double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_D_to_f(name) extern double _D_NAME(name)(const double *x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_s_to_f(name) extern double _D_NAME(name)(const char *x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_dd_to_f(name) extern double _D_NAME(name)(double x, double y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_dl_to_f(name) extern double _D_NAME(name)(double x, long double y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_dD_to_f(name) extern double _D_NAME(name)(double x, double *y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_ddd_to_f(name) extern double _D_NAME(name)(double x, double y, double z) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_dI_to_f(name) extern double _D_NAME(name)(double x, int *y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_ddI_to_f(name) extern double _D_NAME(name)(double x, double y, int *z) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_id_to_f(name) extern double _D_NAME(name)(int n, double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_di_to_f(name) extern double _D_NAME(name)(double x, int n) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_d_dj_to_f(name) extern double _D_NAME(name)(double x, long n) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_i_d_to_f(name) extern int name(double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_j_d_to_f(name) extern long name(double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_k_d_to_f(name) extern long long name(double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_i_dd_to_f(name) extern int name(double x, double y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  define _MATH_ALIAS_v_dDD_to_f(name) extern void name(double x, double *y, double *z) __attribute__((__alias__(_FLOAT_ALIAS(name))));
# else
#  define _MATH_ALIAS_d_to_f(name) double _D_NAME(name)(void) { return (double) __FLOAT_NAME(name)(); }
#  define _MATH_ALIAS_d_d_to_f(name) double _D_NAME(name)(double x) { return (double) __FLOAT_NAME(name)((float) x); }
#  define _MATH_ALIAS_d_D_to_f(name) double _D_NAME(name)(const double *x) { return (double) __FLOAT_NAME(name)((float *) x); }
#  define _MATH_ALIAS_d_s_to_f(name) double _D_NAME(name)(const char *x) { return (double) __FLOAT_NAME(name)(x); }
#  define _MATH_ALIAS_d_dd_to_f(name) double _D_NAME(name)(double x, double y) { return (double) __FLOAT_NAME(name)((float) x, (float) y); }
#  define _MATH_ALIAS_d_dl_to_f(name) double _D_NAME(name)(double x, long double y) { return (double) __FLOAT_NAME(name)((float) x, y); }
#  define _MATH_ALIAS_d_dD_to_f(name) double _D_NAME(name)(double x, double *y) { return (double) __FLOAT_NAME(name)((float) x, (float *) y); }
#  define _MATH_ALIAS_d_ddd_to_f(name) double _D_NAME(name)(double x, double y, double z) { return (double) __FLOAT_NAME(name)((float) x, (float) y, (float) z); }
#  define _MATH_ALIAS_d_dI_to_f(name) double _D_NAME(name)(double x, int *y) { return (double) __FLOAT_NAME(name)((float) x, y); }
#  define _MATH_ALIAS_d_ddI_to_f(name) double _D_NAME(name)(double x, double y, int *z) { return (double) __FLOAT_NAME(name)((float) x, (float) y, z); }
#  define _MATH_ALIAS_d_id_to_f(name) double _D_NAME(name)(int n, double x) { return (double) __FLOAT_NAME(name)(n, (float) x); }
#  define _MATH_ALIAS_d_di_to_f(name) double _D_NAME(name)(double x, int n) { return (double) __FLOAT_NAME(name)((float) x, n); }
#  define _MATH_ALIAS_d_dj_to_f(name) double _D_NAME(name)(double x, long n) { return (double) __FLOAT_NAME(name)((float) x, n); }
#  define _MATH_ALIAS_i_d_to_f(name) int _D_NAME(name)(double x) { return __FLOAT_NAME(name)((float) x); }
#  define _MATH_ALIAS_j_d_to_f(name) long _D_NAME(name)(double x) { return __FLOAT_NAME(name)((float) x); }
#  define _MATH_ALIAS_k_d_to_f(name) long long _D_NAME(name)(double x) { return __FLOAT_NAME(name)((float) x); }
#  define _MATH_ALIAS_i_dd_to_f(name) int _D_NAME(name)(double x, double y) { return __FLOAT_NAME(name)((float) x, (float) y); }
#  define _MATH_ALIAS_v_dDD_to_f(name) void _D_NAME(name)(double x, double *y, double *z) { return __FLOAT_NAME(name)((float) x, (float *) y, (float *) z); }
# endif
#else
# define _MATH_ALIAS_d_to_f(name)
# define _MATH_ALIAS_d_d_to_f(name)
# define _MATH_ALIAS_d_D_to_f(name)
# define _MATH_ALIAS_d_s_to_f(name)
# define _MATH_ALIAS_d_dd_to_f(name)
# define _MATH_ALIAS_d_dl_to_f(name)
# define _MATH_ALIAS_d_dD_to_f(name)
# define _MATH_ALIAS_d_ddd_to_f(name)
# define _MATH_ALIAS_d_dI_to_f(name)
# define _MATH_ALIAS_d_ddI_to_f(name)
# define _MATH_ALIAS_d_id_to_f(name)
# define _MATH_ALIAS_d_di_to_f(name)
# define _MATH_ALIAS_d_dj_to_f(name)
# define _MATH_ALIAS_i_d_to_f(name)
# define _MATH_ALIAS_j_d_to_f(name)
# define _MATH_ALIAS_k_d_to_f(name)
# define _MATH_ALIAS_i_dd_to_f(name)
# define _MATH_ALIAS_v_dDD_to_f(name)
# define _NEED_FLOAT64
#endif

#ifdef _LDBL_EQ_DBL
# ifdef _DOUBLE_IS_32BITS
#  ifdef _HAVE_ALIAS_ATTRIBUTE
#   define _MATH_ALIAS_l_to_f(name) extern long double _LD_NAME(name)(void) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_l_to_f(name) extern long double _LD_NAME(name)(long double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_L_to_f(name) extern long double _LD_NAME(name)(const long double *x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_s_to_f(name) extern long double _LD_NAME(name)(const char *x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_ll_to_f(name) extern long double _LD_NAME(name)(long double x, long double y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_lL_to_f(name) extern long double _LD_NAME(name)(long double x, long double *y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_lll_to_f(name) extern long double _LD_NAME(name)(long double x, long double y, long double z) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_lI_to_f(name) extern long double _LD_NAME(name)(long double x, int *y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_llI_to_f(name) extern long double _LD_NAME(name)(long double x, long double y, int *z) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_il_to_f(name) extern long double _LD_NAME(name)(int n, long double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_li_to_f(name) extern long double _LD_NAME(name)(long double x, int n) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_l_lj_to_f(name) extern long double _LD_NAME(name)(long double x, long n) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_i_l_to_f(name) extern int _LD_NAME(name)(long double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_j_l_to_f(name) extern long _LD_NAME(name)(long double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_k_l_to_f(name) extern long long _LD_NAME(name)(long double x) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_i_ll_to_f(name) extern int _LD_NAME(name)(long double x, long double y) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#   define _MATH_ALIAS_v_lLL_to_f(name) extern void _LD_NAME(name)(long double x, long double *y, long double *z) __attribute__((__alias__(_FLOAT_ALIAS(name))));
#  else
#   define _MATH_ALIAS_l_to_f(name) long double _LD_NAME(name)(void) { return (long double) _FLOAT_NAME(name)(); }
#   define _MATH_ALIAS_l_l_to_f(name) long double _LD_NAME(name)(long double x) { return (long double) _FLOAT_NAME(name)((float) x); }
#   define _MATH_ALIAS_l_L_to_f(name) long double _LD_NAME(name)(const long double *x) { return (long double) _FLOAT_NAME(name)((float *) x); }
#   define _MATH_ALIAS_l_s_to_f(name) long double _LD_NAME(name)(const char *x) { return (long double) _FLOAT_NAME(name)(x); }
#   define _MATH_ALIAS_l_ll_to_f(name) long double _LD_NAME(name)(long double x, long double y) { return (long double) _FLOAT_NAME(name)((float) x, (float) y); }
#   define _MATH_ALIAS_l_lL_to_f(name) long double _LD_NAME(name)(long double x, long double *y) { return (long double) _FLOAT_NAME(name)((float) x, (float *) y); }
#   define _MATH_ALIAS_l_lll_to_f(name) long double _LD_NAME(name)(long double x, long double y, long double z) { return (long double) _FLOAT_NAME(name)((float) x, (float) y, (float) z); }
#   define _MATH_ALIAS_l_lI_to_f(name) long double _LD_NAME(name)(long double x, int *y) { return (long double) _FLOAT_NAME(name)((float) x, y); }
#   define _MATH_ALIAS_l_llI_to_f(name) long double _LD_NAME(name)(long double x, long double y, int *z) { return (long double) _FLOAT_NAME(name)((float) x, (float) y, z); }
#   define _MATH_ALIAS_l_il_to_f(name) long double _LD_NAME(name)(int n, long double x) { return (long double) _FLOAT_NAME(name)(n, (float) x); }
#   define _MATH_ALIAS_l_li_to_f(name) long double _LD_NAME(name)(long double x, int n) { return (long double) _FLOAT_NAME(name)((float) x, n); }
#   define _MATH_ALIAS_l_lj_to_f(name) long double _LD_NAME(name)(long double x, long n) { return (long double) _FLOAT_NAME(name)((float) x, n); }
#   define _MATH_ALIAS_i_l_to_f(name) int _LD_NAME(name)(long double x) { return _FLOAT_NAME(name)((float) x); }
#   define _MATH_ALIAS_j_l_to_f(name) long _LD_NAME(name)(long double x) { return _FLOAT_NAME(name)((float) x); }
#   define _MATH_ALIAS_k_l_to_f(name) long long _LD_NAME(name)(long double x) { return _FLOAT_NAME(name)((float) x); }
#   define _MATH_ALIAS_i_ll_to_f(name) int _LD_NAME(name)(long double x, long double y) { return _FLOAT_NAME(name)((float) x, (float) y); }
#   define _MATH_ALIAS_v_lLL_to_f(name) void _LD_NAME(name)(long double x, long double *y, long double *z) { return _FLOAT_NAME(name)((float) x, (float *) y, (float *) z); }
#  endif
#  define _MATH_ALIAS_l_to_d(name)
#  define _MATH_ALIAS_l_l_to_d(name)
#  define _MATH_ALIAS_l_L_to_d(name)
#  define _MATH_ALIAS_l_s_to_d(name)
#  define _MATH_ALIAS_l_ll_to_d(name)
#  define _MATH_ALIAS_l_lL_to_d(name)
#  define _MATH_ALIAS_l_lll_to_d(name)
#  define _MATH_ALIAS_l_lI_to_d(name)
#  define _MATH_ALIAS_l_llI_to_d(name)
#  define _MATH_ALIAS_l_il_to_d(name)
#  define _MATH_ALIAS_l_li_to_d(name)
#  define _MATH_ALIAS_l_lj_to_d(name)
#  define _MATH_ALIAS_i_l_to_d(name)
#  define _MATH_ALIAS_j_l_to_d(name)
#  define _MATH_ALIAS_k_l_to_d(name)
#  define _MATH_ALIAS_i_ll_to_d(name)
# else
#  define _MATH_ALIAS_l_to_f(name)
#  define _MATH_ALIAS_l_l_to_f(name)
#  define _MATH_ALIAS_l_L_to_f(name)
#  define _MATH_ALIAS_l_s_to_f(name)
#  define _MATH_ALIAS_l_ll_to_f(name)
#  define _MATH_ALIAS_l_lL_to_f(name)
#  define _MATH_ALIAS_l_lll_to_f(name)
#  define _MATH_ALIAS_l_lI_to_f(name)
#  define _MATH_ALIAS_l_llI_to_f(name)
#  define _MATH_ALIAS_l_il_to_f(name)
#  define _MATH_ALIAS_l_li_to_f(name)
#  define _MATH_ALIAS_l_lj_to_f(name)
#  define _MATH_ALIAS_i_l_to_f(name)
#  define _MATH_ALIAS_j_l_to_f(name)
#  define _MATH_ALIAS_k_l_to_f(name)
#  define _MATH_ALIAS_i_ll_to_f(name)
#  define _MATH_ALIAS_v_lLL_to_f(name)
#  ifdef _HAVE_ALIAS_ATTRIBUTE
#   define _MATH_ALIAS_l_to_d(name) extern long double _LD_NAME(name)(void) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_l_to_d(name) extern long double _LD_NAME(name)(long double x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_L_to_d(name) extern long double _LD_NAME(name)(const long double *x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_s_to_d(name) extern long double _LD_NAME(name)(const char *x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_ll_to_d(name) extern long double _LD_NAME(name)(long double x, long double y) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_lL_to_d(name) extern long double _LD_NAME(name)(long double x, long double *y) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_lll_to_d(name) extern long double _LD_NAME(name)(long double x, long double y, long double z) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_lI_to_d(name) extern long double _LD_NAME(name)(long double x, int *y) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_llI_to_d(name) extern long double _LD_NAME(name)(long double x, long double y, int *z) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_il_to_d(name) extern long double _LD_NAME(name)(int n, long double x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_li_to_d(name) extern long double _LD_NAME(name)(long double x, int n) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_l_lj_to_d(name) extern long double _LD_NAME(name)(long double x, long n) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_i_l_to_d(name) extern int _LD_NAME(name)(long double x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_j_l_to_d(name) extern long _LD_NAME(name)(long double x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_k_l_to_d(name) extern long long _LD_NAME(name)(long double x) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_i_ll_to_d(name) extern int _LD_NAME(name)(long double x, long double y) __attribute__((__alias__(_D_ALIAS(name))));
#   define _MATH_ALIAS_v_lLL_to_d(name) extern void _LD_NAME(name)(long double x, long double *y, long double *z) __attribute__((__alias__(_D_ALIAS(name))));
#  else
#   define _MATH_ALIAS_l_to_d(name) long double _LD_NAME(name)(void) { return (long double) _D_NAME(name)(); }
#   define _MATH_ALIAS_l_l_to_d(name) long double _LD_NAME(name)(long double x) { return (long double) _D_NAME(name)((double) x); }
#   define _MATH_ALIAS_l_L_to_d(name) long double _LD_NAME(name)(const long double *x) { return (long double) _D_NAME(name)((double *) x); }
#   define _MATH_ALIAS_l_s_to_d(name) long double _LD_NAME(name)(const char *x) { return (long double) _D_NAME(name)(x); }
#   define _MATH_ALIAS_l_ll_to_d(name) long double _LD_NAME(name)(long double x, long double y) { return (long double) _D_NAME(name)((double) x, (double) y); }
#   define _MATH_ALIAS_l_lL_to_d(name) long double _LD_NAME(name)(long double x, long double *y) { return (long double) _D_NAME(name)((double) x, (double *) y); }
#   define _MATH_ALIAS_l_lll_to_d(name) long double _LD_NAME(name)(long double x, long double y, long double z) { return (long double) _D_NAME(name)((double) x, (double) y, (double) z); }
#   define _MATH_ALIAS_l_lI_to_d(name) long double _LD_NAME(name)(long double x, int *y) { return (long double) _D_NAME(name)((double) x, y); }
#   define _MATH_ALIAS_l_llI_to_d(name) long double _LD_NAME(name)(long double x, long double y, int *z) { return (long double) _D_NAME(name)((double) x, (double) y, z); }
#   define _MATH_ALIAS_l_il_to_d(name) long double _LD_NAME(name)(int n, long double x) { return (long double) _D_NAME(name)(n, (double) x); }
#   define _MATH_ALIAS_l_li_to_d(name) long double _LD_NAME(name)(long double x, int n) { return (long double) _D_NAME(name)((double) x, n); }
#   define _MATH_ALIAS_l_lj_to_d(name) long double _LD_NAME(name)(long double x, long j) { return (long double) _D_NAME(name)((double) x, j); }
#   define _MATH_ALIAS_i_l_to_d(name) int _LD_NAME(name)(long double x) { return _D_NAME(name)((double) x); }
#   define _MATH_ALIAS_j_l_to_d(name) long _LD_NAME(name)(long double x) { return _D_NAME(name)((double) x); }
#   define _MATH_ALIAS_k_l_to_d(name) long long _LD_NAME(name)(long double x) { return _D_NAME(name)((double) x); }
#   define _MATH_ALIAS_i_ll_to_d(name) int _LD_NAME(name)(long double x, long double y) { return _D_NAME(name)((double) x, (double) y); }
#   define _MATH_ALIAS_v_lLL_to_d(name) void _LD_NAME(name)(long double x, long double *y, long double *z) { return _D_NAME(name)((double) x, (double *) y, (double *) z); }
#  endif
# endif
#else
# if __SIZEOF_LONG_DOUBLE__ == 8
#  define _NAME_64(x) _LD_NAME_REG(x)
# define _NAME_64_SPECIAL(d,l) l
   typedef long double __float64;
#  define FORCE_FLOAT64 FORCE_LONG_DOUBLE
#  define pick_float64_except(a,b) pick_long_double_except(a,b)
#  define _NEED_FLOAT64
#  define __F_64(x)     x ## L
#  define _F_64(x)      __F_64(x)
#  define _FLOAT64_MIN  LDBL_MIN
#  define _FLOAT64_MAX  LDBL_MAX
#  define force_eval_float64 force_eval_long_double
# endif
# define _MATH_ALIAS_l_to_f(name)
# define _MATH_ALIAS_l_l_to_f(name)
# define _MATH_ALIAS_l_L_to_f(name)
# define _MATH_ALIAS_l_s_to_f(name)
# define _MATH_ALIAS_l_ll_to_f(name)
# define _MATH_ALIAS_l_lL_to_f(name)
# define _MATH_ALIAS_l_lll_to_f(name)
# define _MATH_ALIAS_l_lI_to_f(name)
# define _MATH_ALIAS_l_llI_to_f(name)
# define _MATH_ALIAS_l_il_to_f(name)
# define _MATH_ALIAS_l_li_to_f(name)
# define _MATH_ALIAS_l_lj_to_f(name)
# define _MATH_ALIAS_i_l_to_f(name)
# define _MATH_ALIAS_j_l_to_f(name)
# define _MATH_ALIAS_k_l_to_f(name)
# define _MATH_ALIAS_i_ll_to_f(name)
# define _MATH_ALIAS_v_lLL_to_f(name)
# define _MATH_ALIAS_l_to_d(name)
# define _MATH_ALIAS_l_l_to_d(name)
# define _MATH_ALIAS_l_L_to_d(name)
# define _MATH_ALIAS_l_s_to_d(name)
# define _MATH_ALIAS_l_ll_to_d(name)
# define _MATH_ALIAS_l_lL_to_d(name)
# define _MATH_ALIAS_l_lll_to_d(name)
# define _MATH_ALIAS_l_lI_to_d(name)
# define _MATH_ALIAS_l_llI_to_d(name)
# define _MATH_ALIAS_l_il_to_d(name)
# define _MATH_ALIAS_l_li_to_d(name)
# define _MATH_ALIAS_l_lj_to_d(name)
# define _MATH_ALIAS_i_l_to_d(name)
# define _MATH_ALIAS_j_l_to_d(name)
# define _MATH_ALIAS_k_l_to_d(name)
# define _MATH_ALIAS_i_ll_to_d(name)
# define _MATH_ALIAS_v_lLL_to_d(name)
#endif

#ifndef _NAME_64
# define _NAME_64(x) _D_NAME_REG(x)
# define _NAME_64_SPECIAL(d,l) d
# define _F_64(x) x
# define _FLOAT64_MIN DBL_MIN
# define _FLOAT64_MAX DBL_MAX
typedef double __float64;
# define FORCE_FLOAT64 FORCE_DOUBLE
# define pick_float64_except(a,b) pick_double_except(a,b)
# define force_eval_float64 force_eval_double
#endif

#if __SIZEOF_LONG_DOUBLE__ > 8
# define _NEED_FLOAT_HUGE
/* Guess long double layout based on compiler defines */

#if __LDBL_MANT_DIG__ == 64 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384
#define _INTEL80_FLOAT
#ifdef __LP64__
#define _INTEL80_FLOAT_PAD
#endif
#endif

#if __LDBL_MANT_DIG__ == 113 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384
#define _IEEE128_FLOAT
#endif

#if __LDBL_MANT_DIG__ == 106 && __LDBL_MAX_EXP__ == 1024
#define _DOUBLE_DOUBLE_FLOAT
#endif

#ifndef __FLOAT_WORD_ORDER__
#define __FLOAT_WORD_ORDER__     __BYTE_ORDER__
#endif

#endif

#define _MATH_ALIAS_f(name) \
    _MATH_ALIAS_d_to_f(name) \
    _MATH_ALIAS_l_to_f(name)

#define _MATH_ALIAS_f_f(name) \
    _MATH_ALIAS_d_d_to_f(name) \
    _MATH_ALIAS_l_l_to_f(name)

#define _MATH_ALIAS_f_F(name) \
    _MATH_ALIAS_d_D_to_f(name) \
    _MATH_ALIAS_l_L_to_f(name)

#define _MATH_ALIAS_f_s(name) \
    _MATH_ALIAS_d_s_to_f(name) \
    _MATH_ALIAS_l_s_to_f(name)

#define _MATH_ALIAS_f_ff(name) \
    _MATH_ALIAS_d_dd_to_f(name) \
    _MATH_ALIAS_l_ll_to_f(name)

#define _MATH_ALIAS_f_fl(name) \
    _MATH_ALIAS_d_dl_to_f(name) \
    _MATH_ALIAS_l_ll_to_f(name)

#define _MATH_ALIAS_f_fF(name) \
    _MATH_ALIAS_d_dD_to_f(name) \
    _MATH_ALIAS_l_lL_to_f(name)

#define _MATH_ALIAS_f_fff(name) \
    _MATH_ALIAS_d_ddd_to_f(name) \
    _MATH_ALIAS_l_lll_to_f(name)

#define _MATH_ALIAS_f_fI(name) \
    _MATH_ALIAS_d_dI_to_f(name) \
    _MATH_ALIAS_l_lI_to_f(name)

#define _MATH_ALIAS_f_ffI(name) \
    _MATH_ALIAS_d_ddI_to_f(name) \
    _MATH_ALIAS_l_llI_to_f(name)

#define _MATH_ALIAS_f_if(name) \
    _MATH_ALIAS_d_id_to_f(name) \
    _MATH_ALIAS_l_il_to_f(name)

#define _MATH_ALIAS_f_fi(name) \
    _MATH_ALIAS_d_di_to_f(name) \
    _MATH_ALIAS_l_li_to_f(name)

#define _MATH_ALIAS_f_fj(name) \
    _MATH_ALIAS_d_dj_to_f(name) \
    _MATH_ALIAS_l_lj_to_f(name)

#define _MATH_ALIAS_i_f(name) \
    _MATH_ALIAS_i_d_to_f(name) \
    _MATH_ALIAS_i_l_to_f(name)

#define _MATH_ALIAS_j_f(name) \
    _MATH_ALIAS_j_d_to_f(name) \
    _MATH_ALIAS_j_l_to_f(name)

#define _MATH_ALIAS_k_f(name) \
    _MATH_ALIAS_k_d_to_f(name) \
    _MATH_ALIAS_k_l_to_f(name)

#define _MATH_ALIAS_i_ff(name) \
    _MATH_ALIAS_i_dd_to_f(name) \
    _MATH_ALIAS_i_ll_to_f(name)

#define _MATH_ALIAS_v_fFF(name) \
    _MATH_ALIAS_v_dDD_to_f(name) \
    _MATH_ALIAS_v_lLL_to_f(name)

#define _MATH_ALIAS_d(name) \
    _MATH_ALIAS_l_to_d(name)

#define _MATH_ALIAS_d_d(name) \
    _MATH_ALIAS_l_l_to_d(name)

#define _MATH_ALIAS_d_D(name) \
    _MATH_ALIAS_l_L_to_d(name)

#define _MATH_ALIAS_d_s(name) \
    _MATH_ALIAS_l_s_to_d(name)

#define _MATH_ALIAS_d_dd(name) \
    _MATH_ALIAS_l_ll_to_d(name)

#define _MATH_ALIAS_d_dl(name) \
    _MATH_ALIAS_l_ll_to_d(name)

#define _MATH_ALIAS_d_dD(name) \
    _MATH_ALIAS_l_lL_to_d(name)

#define _MATH_ALIAS_d_ddd(name) \
    _MATH_ALIAS_l_lll_to_d(name)

#define _MATH_ALIAS_d_dI(name) \
    _MATH_ALIAS_l_lI_to_d(name)

#define _MATH_ALIAS_d_ddI(name) \
    _MATH_ALIAS_l_llI_to_d(name)

#define _MATH_ALIAS_d_id(name) \
    _MATH_ALIAS_l_il_to_d(name)

#define _MATH_ALIAS_d_di(name) \
    _MATH_ALIAS_l_li_to_d(name)

#define _MATH_ALIAS_d_dj(name) \
    _MATH_ALIAS_l_lj_to_d(name)

#define _MATH_ALIAS_i_d(name) \
    _MATH_ALIAS_i_l_to_d(name)

#define _MATH_ALIAS_j_d(name) \
    _MATH_ALIAS_j_l_to_d(name)

#define _MATH_ALIAS_k_d(name) \
    _MATH_ALIAS_k_l_to_d(name)

#define _MATH_ALIAS_i_dd(name) \
    _MATH_ALIAS_i_ll_to_d(name)

#define _MATH_ALIAS_v_dDD(name) \
    _MATH_ALIAS_v_lLL_to_d(name)

/* Evaluate an expression as the specified type, normally a type
   cast should be enough, but compilers implement non-standard
   excess-precision handling, so when FLT_EVAL_METHOD != 0 then
   these functions may need to be customized.  */
static ALWAYS_INLINE float
eval_as_float (float x)
{
  return x;
}
static ALWAYS_INLINE double
eval_as_double (double x)
{
  return x;
}

/* gcc emitting PE/COFF doesn't support visibility */
#if defined (__GNUC__) && !defined (__CYGWIN__)
# define HIDDEN __attribute__ ((__visibility__ ("hidden")))
#else
# define HIDDEN
#endif

/* Error handling tail calls for special cases, with a sign argument.
   The sign of the return value is set if the argument is non-zero.  */

/* The result overflows.  */
HIDDEN float __math_oflowf (uint32_t);
/* The result underflows to 0 in nearest rounding mode.  */
HIDDEN float __math_uflowf (uint32_t);
/* The result underflows to 0 in some directed rounding mode only.  */
HIDDEN float __math_may_uflowf (uint32_t);
/* Division by zero.  */
HIDDEN float __math_divzerof (uint32_t);
/* The result overflows.  */
HIDDEN __float64 __math_oflow (uint32_t);
/* The result underflows to 0 in nearest rounding mode.  */
HIDDEN __float64 __math_uflow (uint32_t);
/* The result underflows to 0 in some directed rounding mode only.  */
HIDDEN __float64 __math_may_uflow (uint32_t);
/* Division by zero.  */
HIDDEN __float64 __math_divzero (uint32_t);

/* Error handling using input checking.  */

/* Invalid input unless it is a quiet NaN.  */
HIDDEN float __math_invalidf (float);
/* set invalid exception */
#if defined(FE_INVALID) && !defined(PICOLIBC_FLOAT_NOEXECPT)
HIDDEN void __math_set_invalidf(void);
#else
#define __math_set_invalidf()   ((void) 0)
#endif
/* Invalid input unless it is a quiet NaN.  */
HIDDEN __float64 __math_invalid (__float64);
/* set invalid exception */
#if defined(FE_INVALID) && !defined(PICOLIBC_DOUBLE_NOEXECPT)
HIDDEN void __math_set_invalid(void);
#else
#define __math_set_invalid()    ((void) 0)
#endif

#ifdef _HAVE_LONG_DOUBLE
HIDDEN long double __math_oflowl (uint32_t);
/* The result underflows to 0 in nearest rounding mode.  */
HIDDEN long double __math_uflowl (uint32_t);
/* The result underflows to 0 in some directed rounding mode only.  */
HIDDEN long double __math_may_uflowl (uint32_t);
/* Division by zero.  */
HIDDEN long double __math_divzerol (uint32_t);
/* Invalid input unless it is a quiet NaN.  */
HIDDEN long double __math_invalidl (long double);
/* set invalid exception */
#if defined(FE_INVALID) && !defined(PICOLIBC_LONG_DOUBLE_NOEXECPT)
HIDDEN void __math_set_invalidl(void);
#else
#define __math_set_invalidl()    ((void) 0)
#endif
#endif

/* Error handling using output checking, only for errno setting.  */

/* Check if the result overflowed to infinity.  */
HIDDEN float __math_check_oflowf (float);
/* Check if the result overflowed to infinity.  */
HIDDEN __float64 __math_check_oflow (__float64);
#ifdef _NEED_FLOAT_HUGE
HIDDEN long double __math_check_oflowl(long double);
#endif

/* Check if the result underflowed to 0.  */
HIDDEN float __math_check_uflowf (float);
HIDDEN __float64 __math_check_uflow (__float64);
#ifdef _NEED_FLOAT_HUGE
HIDDEN long double __math_check_uflowl(long double);
#endif

/* Check if the result overflowed to infinity.  */
static inline __float64
check_oflow (__float64 x)
{
  return WANT_ERRNO ? __math_check_oflow (x) : x;
}

/* Check if the result overflowed to infinity.  */
static inline float
check_oflowf (float x)
{
  return WANT_ERRNO ? __math_check_oflowf (x) : x;
}

#ifdef _NEED_FLOAT_HUGE
static inline long double
check_oflowl (long double x)
{
  return WANT_ERRNO ? __math_check_oflowl (x) : x;
}
#endif

/* Check if the result underflowed to 0.  */
static inline __float64
check_uflow (__float64 x)
{
  return WANT_ERRNO ? __math_check_uflow (x) : x;
}

/* Check if the result underflowed to 0.  */
static inline float
check_uflowf (float x)
{
  return WANT_ERRNO ? __math_check_uflowf (x) : x;
}

#ifdef _NEED_FLOAT_HUGE
static inline long double
check_uflowl (long double x)
{
  return WANT_ERRNO ? __math_check_uflowl (x) : x;
}
#endif

/* Set inexact exception */
#if defined(FE_INEXACT) && !defined(PICOLIBC_DOUBLE_NOEXECPT)
__float64 __math_inexact(__float64);
void __math_set_inexact(void);
#else
#define __math_inexact(val) (val)
#define __math_set_inexact()    ((void) 0)
#endif

#if defined(FE_INEXACT) && !defined(PICOLIBC_FLOAT_NOEXECPT)
float __math_inexactf(float val);
void __math_set_inexactf(void);
#else
#define __math_inexactf(val) (val)
#define __math_set_inexactf()   ((void) 0)
#endif

#if defined(FE_INEXACT) && !defined(PICOLIBC_LONG_DOUBLE_NOEXECPT) && defined(_NEED_FLOAT_HUGE)
long double __math_inexactl(long double val);
void __math_set_inexactl(void);
#else
#define __math_inexactl(val) (val)
#define __math_set_inexactl()   ((void) 0)
#endif

/* Shared between expf, exp2f and powf.  */
#define EXP2F_TABLE_BITS 5
#define EXP2F_POLY_ORDER 3
extern const struct exp2f_data
{
  uint64_t tab[1 << EXP2F_TABLE_BITS];
  double shift_scaled;
  double poly[EXP2F_POLY_ORDER];
  double shift;
  double invln2_scaled;
  double poly_scaled[EXP2F_POLY_ORDER];
} __exp2f_data HIDDEN;

#define LOGF_TABLE_BITS 4
#define LOGF_POLY_ORDER 4
extern const struct logf_data
{
  struct
  {
    double invc, logc;
  } tab[1 << LOGF_TABLE_BITS];
  double ln2;
  double poly[LOGF_POLY_ORDER - 1]; /* First order coefficient is 1.  */
} __logf_data HIDDEN;

#define LOG2F_TABLE_BITS 4
#define LOG2F_POLY_ORDER 4
extern const struct log2f_data
{
  struct
  {
    double invc, logc;
  } tab[1 << LOG2F_TABLE_BITS];
  double poly[LOG2F_POLY_ORDER];
} __log2f_data HIDDEN;

#define POWF_LOG2_TABLE_BITS 4
#define POWF_LOG2_POLY_ORDER 5
#if TOINT_INTRINSICS
# define POWF_SCALE_BITS EXP2F_TABLE_BITS
#else
# define POWF_SCALE_BITS 0
#endif
#define POWF_SCALE ((double) (1 << POWF_SCALE_BITS))
extern const struct powf_log2_data
{
  struct
  {
    double invc, logc;
  } tab[1 << POWF_LOG2_TABLE_BITS];
  double poly[POWF_LOG2_POLY_ORDER];
} __powf_log2_data HIDDEN;

#define EXP_TABLE_BITS 7
#define EXP_POLY_ORDER 5
/* Use polynomial that is optimized for a wider input range.  This may be
   needed for good precision in non-nearest rounding and !TOINT_INTRINSICS.  */
#define EXP_POLY_WIDE 0
/* Use close to nearest rounding toint when !TOINT_INTRINSICS.  This may be
   needed for good precision in non-nearest rouning and !EXP_POLY_WIDE.  */
#define EXP_USE_TOINT_NARROW 0
#define EXP2_POLY_ORDER 5
#define EXP2_POLY_WIDE 0
extern const struct exp_data
{
  double invln2N;
  double shift;
  double negln2hiN;
  double negln2loN;
  double poly[4]; /* Last four coefficients.  */
  double exp2_shift;
  double exp2_poly[EXP2_POLY_ORDER];
  uint64_t tab[2*(1 << EXP_TABLE_BITS)];
} __exp_data HIDDEN;

#define LOG_TABLE_BITS 7
#define LOG_POLY_ORDER 6
#define LOG_POLY1_ORDER 12
extern const struct log_data
{
  double ln2hi;
  double ln2lo;
  double poly[LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
  double poly1[LOG_POLY1_ORDER - 1];
  struct {double invc, logc;} tab[1 << LOG_TABLE_BITS];
#if !_HAVE_FAST_FMA
  struct {double chi, clo;} tab2[1 << LOG_TABLE_BITS];
#endif
} __log_data HIDDEN;

#define LOG2_TABLE_BITS 6
#define LOG2_POLY_ORDER 7
#define LOG2_POLY1_ORDER 11
extern const struct log2_data
{
  double invln2hi;
  double invln2lo;
  double poly[LOG2_POLY_ORDER - 1];
  double poly1[LOG2_POLY1_ORDER - 1];
  struct {double invc, logc;} tab[1 << LOG2_TABLE_BITS];
#if !_HAVE_FAST_FMA
  struct {double chi, clo;} tab2[1 << LOG2_TABLE_BITS];
#endif
} __log2_data HIDDEN;

#define POW_LOG_TABLE_BITS 7
#define POW_LOG_POLY_ORDER 8
extern const struct pow_log_data
{
  double ln2hi;
  double ln2lo;
  double poly[POW_LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
  /* Note: the pad field is unused, but allows slightly faster indexing.  */
  struct {double invc, pad, logc, logctail;} tab[1 << POW_LOG_TABLE_BITS];
} __pow_log_data HIDDEN;

#if WANT_ERRNO
HIDDEN double
__math_with_errno (double y, int e);

HIDDEN float
__math_with_errnof (float y, int e);

#ifdef _HAVE_LONG_DOUBLE
HIDDEN long double
__math_with_errnol (long double y, int e);
#endif
#else
#define __math_with_errno(x, e) (x)
#define __math_with_errnof(x, e) (x)
#define __math_with_errnol(x, e) (x)
#endif

/* Check if the result is a denorm. */
#if (defined(FE_UNDERFLOW) && !defined(PICOLIBC_FLOAT_NOEXCEPT)) || WANT_ERRNO
float __math_denormf (float x);
#else
#define __math_denormf(x) (x)
#endif

#if (defined(FE_UNDERFLOW) && !defined(PICOLIBC_DOUBLE_NOEXCEPT)) || WANT_ERRNO
__float64 __math_denorm (__float64 x);
#else
#define __math_denorm(x) (x)
#endif

#if defined(_NEED_FLOAT_HUGE) && ((defined(FE_UNDERFLOW) && !defined(PICOLIBC_LONG_DOUBLE_NOEXCEPT)) || WANT_ERRNO)
long double __math_denorml(long double x);
#else
#define __math_denorml(x) (x)
#endif

HIDDEN double
__math_xflow (uint32_t sign, double y);

HIDDEN float
__math_xflowf (uint32_t sign, float y);

HIDDEN __float64
__math_lgamma_r (__float64 y, int *signgamp, int *divzero);

HIDDEN float
__math_lgammaf_r (float y, int *signgamp, int *divzero);

#ifdef __weak_reference
extern int __signgam;
#else
#define __signgam signgam
#endif

/* Make aliases for 64-bit functions to the correct name */
#define acos64 _NAME_64(acos)
#define acosh64 _NAME_64(acosh)
#define asin64 _NAME_64(asin)
#define asinh64 _NAME_64(asinh)
#define atan64 _NAME_64(atan)
#define atanh64 _NAME_64(atanh)
#define atan264 _NAME_64(atan2)
#define cbrt64 _NAME_64(cbrt)
#define ceil64 _NAME_64(ceil)
#define copysign64 _NAME_64(copysign)
#define cos64 _NAME_64(cos)
#define _cos64 _NAME_64(_cos)
#define cosh64 _NAME_64(cosh)
#define drem64 _NAME_64(drem)
#define erf64 _NAME_64(erf)
#define erfc64 _NAME_64(erfc)
#define exp64 _NAME_64(exp)
#define exp264 _NAME_64(exp2)
#define exp1064 _NAME_64(exp10)
#define expm164 _NAME_64(expm1)
#define fabs64 _NAME_64(fabs)
#define fdim64 _NAME_64(fdim)
#define finite64 _NAME_64(finite)
#define __finite64 _NAME_64(__finite)
#define floor64 _NAME_64(floor)
#define fma64 _NAME_64(fma)
#define fmax64 _NAME_64(fmax)
#define fmin64 _NAME_64(fmin)
#define fmod64 _NAME_64(fmod)
#define __fpclassify64 _NAME_64_SPECIAL(__fpclassifyd, __fpclassifyl)
#define frexp64 _NAME_64(frexp)
#define gamma64 _NAME_64(gamma)
#define getpayload64 _NAME_64(getpayload)
#define hypot64 _NAME_64(hypot)
#define ilogb64 _NAME_64(ilogb)
#define infinity64 _NAME_64(infinity)
#define __iseqsig64 _NAME_64_SPECIAL(__iseqsigd, __iseqsigl)
#define isfinite64 _NAME_64(isfinite)
#define isinf64 _NAME_64(isinf)
#define __isinf64 _NAME_64_SPECIAL(__isinfd, __isinfl)
#define isnan64 _NAME_64(isnan)
#define __isnan64 _NAME_64_SPECIAL(__isnand, __isnanl)
#define ldexp64 _NAME_64(ldexp)
#define j064 _NAME_64(j0)
#define y064 _NAME_64(y0)
#define j164 _NAME_64(j1)
#define y164 _NAME_64(y1)
#define jn64 _NAME_64(jn)
#define yn64 _NAME_64(yn)
#define lgamma64 _NAME_64(lgamma)
#define lgamma64_r _NAME_64_SPECIAL(lgamma_r, lgammal_r)
#define llrint64 _NAME_64(llrint)
#define llround64 _NAME_64(llround)
#define log64 _NAME_64(log)
#define log1064 _NAME_64(log10)
#define log1p64 _NAME_64(log1p)
#define log264 _NAME_64(log2)
#define logb64 _NAME_64(logb)
#define lrint64 _NAME_64(lrint)
#define lround64 _NAME_64(lround)
#define modf64 _NAME_64(modf)
#define nan64 _NAME_64(nan)
#define nearbyint64 _NAME_64(nearbyint)
#define nextafter64 _NAME_64(nextafter)
#define nexttoward64 _NAME_64(nexttoward)
#define pow64 _NAME_64(pow)
#define _pow64 _NAME_64(_pow)
#define pow1064 _NAME_64(pow10)
#define remainder64 _NAME_64(remainder)
#define remquo64 _NAME_64(remquo)
#define rint64 _NAME_64(rint)
#define round64 _NAME_64(round)
#define scalb64 _NAME_64(scalb)
#define scalbn64 _NAME_64(scalbn)
#define scalbln64 _NAME_64(scalbln)
#define significand64 _NAME_64(significand)
#define sin64 _NAME_64(sin)
#define _sin64 _NAME_64(_sin)
#define sincos64 _NAME_64(sincos)
#define sinh64 _NAME_64(sinh)
#define sqrt64 _NAME_64(sqrt)
#define tan64 _NAME_64(tan)
#define tanh64 _NAME_64(tanh)
#define tgamma64 _NAME_64(tgamma)
#define trunc64 _NAME_64(trunc)

#ifdef _HAVE_ALIAS_ATTRIBUTE
float _powf(float, float);
float _sinf(float);
float _cosf(float);

__float64 _pow64(__float64, __float64);
__float64 _sin64(__float64);
__float64 _cos64(__float64);

#ifdef _HAVE_LONG_DOUBLE_MATH
long double _powl(long double, long double);
long double _sinl(long double);
long double _cosl(long double);
#endif

#else

#define _powf(x,y) powf(x,y)
#define _sinf(x) sinf(x)
#define _cosf(x) cosf(x)

#undef _pow64
#undef _sin64
#undef _cos64
#define _pow64(x,y) pow64(x,y)
#define _sin64(x) sin64(x)
#define _cos64(x) cos64(x)

#ifdef _HAVE_LONG_DOUBLE_MATH
#define _powl(x,y) powl(x,y)
#define _sinl(x) sinl(x)
#define _cosl(x) cosl(x)
#endif /* _HAVE_LONG_DOUBLE_MATH */

#endif /* _HAVE_ALIAS_ATTRIBUTE */

#endif
