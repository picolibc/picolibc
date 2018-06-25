/* Configuration for math routines.
   Copyright (c) 2017-2018 Arm Ltd.  All rights reserved.

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

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS AND ANY EXPRESS OR IMPLIED
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

#ifndef WANT_ROUNDING
/* Correct special case results in non-nearest rounding modes.  */
# define WANT_ROUNDING 1
#endif
#ifndef WANT_ERRNO
/* Set errno according to ISO C with (math_errhandling & MATH_ERRNO) != 0.  */
# define WANT_ERRNO 1
#endif
#ifndef WANT_ERRNO_UFLOW
/* Set errno to ERANGE if result underflows to 0 (in all rounding modes).  */
# define WANT_ERRNO_UFLOW (WANT_ROUNDING && WANT_ERRNO)
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
# define TOINT_INTRINSICS 1

static inline double_t
roundtoint (double_t x)
{
  return round (x);
}

static inline uint64_t
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
#ifndef TOINT_RINT
# define TOINT_RINT 0
#endif
#ifndef TOINT_SHIFT
# define TOINT_SHIFT 1
#endif

static inline uint32_t
asuint (float f)
{
  union
  {
    float f;
    uint32_t i;
  } u = {f};
  return u.i;
}

static inline float
asfloat (uint32_t i)
{
  union
  {
    uint32_t i;
    float f;
  } u = {i};
  return u.f;
}

static inline uint64_t
asuint64 (double f)
{
  union
  {
    double f;
    uint64_t i;
  } u = {f};
  return u.i;
}

static inline double
asdouble (uint64_t i)
{
  union
  {
    uint64_t i;
    double f;
  } u = {i};
  return u.f;
}

#ifndef IEEE_754_2008_SNAN
# define IEEE_754_2008_SNAN 1
#endif
static inline int
issignalingf_inline (float x)
{
  uint32_t ix = asuint (x);
  if (!IEEE_754_2008_SNAN)
    return (ix & 0x7fc00000) == 0x7fc00000;
  return 2 * (ix ^ 0x00400000) > 2u * 0x7fc00000;
}

/* Force the evaluation of a floating-point expression for its side-effect.  */
#if __aarch64__ && __GNUC__
static inline void
force_eval_float (float x)
{
  __asm__ __volatile__ ("" : "+w" (x));
}
static inline void
force_eval_double (double x)
{
  __asm__ __volatile__ ("" : "+w" (x));
}
#else
static inline void
force_eval_float (float x)
{
  volatile float y = x;
}
static inline void
force_eval_double (double x)
{
  volatile double y = x;
}
#endif

/* Evaluate an expression as the specified type, normally a type
   cast should be enough, but compilers implement non-standard
   excess-precision handling, so when FLT_EVAL_METHOD != 0 then
   these functions may need to be customized.  */
static inline float
eval_as_float (float x)
{
  return x;
}
static inline double
eval_as_double (double x)
{
  return x;
}

#ifdef __GNUC__
# define HIDDEN __attribute__ ((__visibility__ ("hidden")))
# define NOINLINE __attribute__ ((noinline))
# define likely(x) __builtin_expect (!!(x), 1)
# define unlikely(x) __builtin_expect (x, 0)
#else
# define HIDDEN
# define NOINLINE
# define likely(x) (x)
# define unlikely(x) (x)
#endif

HIDDEN float __math_oflowf (uint32_t);
HIDDEN float __math_uflowf (uint32_t);
HIDDEN float __math_may_uflowf (uint32_t);
HIDDEN float __math_divzerof (uint32_t);
HIDDEN float __math_invalidf (float);

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

#endif
