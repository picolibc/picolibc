/* Single-precision 2^x function.
   Copyright (c) 2017 Arm Ltd.  All rights reserved.

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

#include "fdlibm.h"
#if !__OBSOLETE_MATH_FLOAT

#include <math.h>
#include <stdint.h>
#include "math_config.h"

/*
EXP2F_TABLE_BITS = 5
EXP2F_POLY_ORDER = 3

ULP error: 0.502 (nearest rounding.)
Relative error: 1.69 * 2^-34 in [-1/64, 1/64] (before rounding.)
Wrong count: 168353 (all nearest rounding wrong results with fma.)
Non-nearest ULP error: 1 (rounded ULP error)
*/

#define N (1 << EXP2F_TABLE_BITS)
#define T __exp2f_data.tab
#define C __exp2f_data.poly
#define SHIFT __exp2f_data.shift_scaled

static inline uint32_t
top12 (float x)
{
  return asuint (x) >> 20;
}

float
exp2f (float x)
{
  uint32_t abstop;
  uint64_t ki, t;
  /* double_t for better performance on targets with FLT_EVAL_METHOD==2.  */
  double_t kd, xd, z, r, r2, y, s;

  xd = (double_t) x;
  abstop = top12 (x) & 0x7ff;
  if (__builtin_expect (abstop >= top12 (128.0f), 0))
    {
      /* |x| >= 128 or x is nan.  */
      if (asuint (x) == asuint (-INFINITY))
	return 0.0f;
      if (abstop >= top12 (INFINITY))
	return x + x;
      if (x > 0.0f)
	return __math_oflowf (0);
      if (x <= -150.0f)
	return __math_uflowf (0);
#if WANT_ERRNO_UFLOW
      if (x < -149.0f)
	return __math_may_uflowf (0);
#endif
    }

  /* x = k/N + r with r in [-1/(2N), 1/(2N)] and int k.  */
  kd = (double) (xd + SHIFT); /* Rounding to double precision is required.  */
  ki = asuint64 (kd);
  kd -= SHIFT; /* k/N for int k.  */
  r = xd - kd;

  /* exp2(x) = 2^(k/N) * 2^r ~= s * (C0*r^3 + C1*r^2 + C2*r + 1) */
  t = T[ki % N];
  t += ki << (52 - EXP2F_TABLE_BITS);
  s = asfloat64 (t);
  z = C[0] * r + C[1];
  r2 = r * r;
  y = C[2] * r + 1;
  y = z * r2 + y;
  y = y * s;
  return (float) y;
}
#endif /* !__OBSOLETE_MATH_FLOAT */
