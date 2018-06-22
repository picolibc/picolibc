/* Double-precision math error handling.
   Copyright (c) 2018 ARM Ltd.  All rights reserved.

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

   THIS SOFTWARE IS PROVIDED BY Arm LTD ``AS IS AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL Arm LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "math_config.h"

#if WANT_ERRNO
#include <errno.h>
/* NOINLINE reduces code size and avoids making math functions non-leaf
   when the error handling is inlined.  */
NOINLINE static double
with_errno (double y, int e)
{
  errno = e;
  return y;
}
#else
#define with_errno(x, e) (x)
#endif

/* NOINLINE reduces code size.  */
NOINLINE static double
xflow (uint32_t sign, double y)
{
  y = opt_barrier_double (sign ? -y : y) * y;
  return with_errno (y, ERANGE);
}

HIDDEN double
__math_uflow (uint32_t sign)
{
  return xflow (sign, 0x1p-767);
}

#if WANT_ERRNO_UFLOW
/* Underflows to zero in some non-nearest rounding mode, setting errno
   is valid even if the result is non-zero, but in the subnormal range.  */
HIDDEN double
__math_may_uflow (uint32_t sign)
{
  return xflow (sign, 0x1.8p-538);
}
#endif

HIDDEN double
__math_oflow (uint32_t sign)
{
  return xflow (sign, 0x1p769);
}

HIDDEN double
__math_divzero (uint32_t sign)
{
  double y = opt_barrier_double (sign ? -1.0 : 1.0) / 0.0;
  return with_errno (y, ERANGE);
}

HIDDEN double
__math_invalid (double x)
{
  double y = (x - x) / (x - x);
  return isnan (x) ? y : with_errno (y, EDOM);
}

/* Check result and set errno if necessary.  */

HIDDEN double
__math_check_uflow (double y)
{
  return y == 0.0 ? with_errno (y, ERANGE) : y;
}

HIDDEN double
__math_check_oflow (double y)
{
  return isinf (y) ? with_errno (y, ERANGE) : y;
}
