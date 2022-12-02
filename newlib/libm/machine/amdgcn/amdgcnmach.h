/*
 * Copyright 2023 Siemens
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/* Common header file for AMD GCN vector math routines.  */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/* Copyright (c) 2017-2018 Arm Ltd.  All rights reserved.

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

/* This header is partially based on:

   newlib/libm/common/fdlibm.h
   newlib/libm/mathfp/zmath.h
   newlib/libm/common/math_errf.c
   newlib/libm/common/math_config.h  */

#include <errno.h>
#include <sys/types.h>
#include <machine/ieeefp.h>
#include "amdgcn_veclib.h"

/* Vectorized versions of macros from newlib/libm/common/fdlibm.h  */

#define EXTRACT_WORDS(hi, lo, x) \
do { \
  vector_union __tmp; \
  __tmp.t_v64df = (x); \
  hi = __builtin_convertvector (__tmp.t_v64di >> 32, typeof (hi)); \
  lo = __builtin_convertvector (__tmp.t_v64di & 0xffffffff, typeof (lo)); \
} while (0)

#define INSERT_WORDS(x, hi, lo, cond) \
do { \
  vector_union __tmp; \
  __tmp.t_v64di = __builtin_convertvector (hi, v64di) << 32 | \
    __builtin_convertvector (lo, v64di) & 0xffffffff; \
  VECTOR_COND_MOVE (x, __tmp.t_v64df, cond); \
} while (0)

#define GET_HIGH_WORD(x, y, cond) \
do { \
  vector_union __tmp; \
  __tmp.t_v64df = (y); \
  VECTOR_COND_MOVE (x, __builtin_convertvector (__tmp.t_v64di >> 32, v64si), \
		    (cond)); \
} while (0)

#define GET_LOW_WORD(x, y, cond) \
do { \
  vector_union __tmp; \
  __tmp.t_v64df = (y); \
  VECTOR_COND_MOVE (x, __builtin_convertvector (__tmp.t_v64di & 0xffffffff, \
						v64si), (cond)); \
} while (0)

#define SET_HIGH_WORD(x, y, cond) \
do { \
  vector_union __tmp; \
  __tmp.t_v64df = x; \
  __tmp.t_v64di &= 0xffffffff; \
  __tmp.t_v64di |= __builtin_convertvector (y, v64di) << 32; \
  VECTOR_COND_MOVE (x, __tmp.t_v64df, (cond)); \
} while (0)

#define SET_LOW_WORD(x, y, cond) \
do { \
  vector_union __tmp; \
  __tmp.t_v64df = x; \
  __tmp.t_v64di &= 0xffffffff00000000ULL; \
  __tmp.t_v64di |= __builtin_convertvector (y, v64di); \
  VECTOR_COND_MOVE (x, __tmp.t_v64df, (cond)); \
 } while (0)

#define GET_FLOAT_WORD(x, y, cond) \
  VECTOR_COND_MOVE(x, CAST_VECTOR(v64si, (y)), (cond))

#define SET_FLOAT_WORD(x, y, cond) \
  VECTOR_COND_MOVE(x, CAST_VECTOR(v64sf, (y)), (cond))

/* Definitions from newlib/libm/common/fdlibm.h  */

#ifdef _FLT_LARGEST_EXPONENT_IS_NORMAL
#define FLT_UWORD_IS_FINITE(x) ((x) == (x))
#define FLT_UWORD_IS_NAN(x) ((x) != (x))
#define FLT_UWORD_IS_INFINITE(x) ((x) != (x))
#define FLT_UWORD_MAX 0x7fffffff
#define FLT_UWORD_EXP_MAX 0x43010000
#define FLT_UWORD_LOG_MAX 0x42b2d4fc
#define FLT_UWORD_LOG_2MAX 0x42b437e0
#define HUGE ((float)0X1.FFFFFEP128)
#else
#define FLT_UWORD_IS_FINITE(x) ((x)<0x7f800000)
#define FLT_UWORD_IS_NAN(x) ((x)>0x7f800000)
#define FLT_UWORD_IS_INFINITE(x) ((x)==0x7f800000)
#define FLT_UWORD_MAX 0x7f7fffffL
#define FLT_UWORD_EXP_MAX 0x43000000
#define FLT_UWORD_LOG_MAX 0x42b17217
#define FLT_UWORD_LOG_2MAX 0x42b2d4fc
#define HUGE ((float)3.40282346638528860e+38)
#endif
#define FLT_UWORD_HALF_MAX (FLT_UWORD_MAX-(1L<<23))
#define FLT_LARGEST_EXP (FLT_UWORD_MAX>>23)

#ifdef _FLT_NO_DENORMALS
#define FLT_UWORD_IS_ZERO(x) ((x)<0x00800000)
#define FLT_UWORD_IS_SUBNORMAL(x) ((x) != (x))
#define FLT_UWORD_MIN 0x00800000
#define FLT_UWORD_EXP_MIN 0x42fc0000
#define FLT_UWORD_LOG_MIN 0x42aeac50
#define FLT_SMALLEST_EXP 1
#else
#define FLT_UWORD_IS_ZERO(x) ((x)==0)
#define FLT_UWORD_IS_SUBNORMAL(x) ((x)<0x00800000)
#define FLT_UWORD_MIN 0x00000001
#define FLT_UWORD_EXP_MIN 0x43160000
#define FLT_UWORD_LOG_MIN 0x42cff1b5
#define FLT_SMALLEST_EXP -22
#endif

/* Definitions from newlib/libm/mathfp/zmath.h  */

#define NUM 3
#define NAN 2
#define INF 1

#define __PI 3.14159265358979323846
#define __SQRT_HALF 0.70710678118654752440
#define __PI_OVER_TWO 1.57079632679489661923132
#define __INV_PI_OVER_TWO_2_24 10680707.430881743590348355907974

typedef const union
{
  unsigned int l[2];
  double d;
} udouble;

typedef const union
{
  unsigned int l;
  float f;
} ufloat;

extern double BIGX;
extern double SMALLX;

extern udouble z_infinity;
extern udouble z_notanum;
extern double  z_rooteps;

extern ufloat  z_infinity_f;
extern ufloat  z_notanum_f;
extern float   z_rooteps_f;

/* Vectorized versions of functions from newlib/libm/common/math_errf.c  */

static v64sf v64sf_math_oflowf (v64si sign)
{
  errno = ERANGE;
  return VECTOR_MERGE (VECTOR_INIT (-0x1p97f),
                       VECTOR_INIT (0x1p97f), sign) * 0x1p97f;
}

static v64sf v64sf_math_uflowf (v64si sign)
{
  errno = ERANGE;
  return VECTOR_MERGE (VECTOR_INIT (-0x1p-95f),
                       VECTOR_INIT (0x1p-95f), sign) * 0x1p-95f;
}

/* Vectorized versions of functions from newlib/libm/common/math_config.h  */

static v64si v64sf_issignalingf_inline (v64sf x)
{
  v64si __mask = VECTOR_INIT (-1);
  v64si ix;
  GET_FLOAT_WORD (ix, x, NO_COND);
  /* Use IEEE-754 2008 encoding - i.e. exponent bits all 1, MSB of
     significand is 0 for signalling NaN.  */
  return ((ix & 0x7f800000) == 0x7f800000) & ((ix & 0x00400000) == 0);
}

/* Vector extensions to sys/reent.h  */

struct v64_reent {
  v64si _v64si_gamma_signgam;
};

extern struct v64_reent *_v64_reent;
#define _V64_REENT _v64_reent

#define _REENT_V64SI_SIGNGAM(ptr)      ((ptr)->_v64si_gamma_signgam)

/* Vector extensions to math.h  */

#define v64si_signgam (*__v64si_signgam())
extern v64si* __v64si_signgam (void);
#define __v64si_signgam_r(ptr) _REENT_V64SI_SIGNGAM(ptr)
