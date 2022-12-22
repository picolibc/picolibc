/*
 * Copyright © 2005-2014 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "math_config.h"

// This is only necessary because the implementation of isnan only works
// properly when long double == double.
// See: https://sourceware.org/ml/newlib/2014/msg00684.html

#if !defined(_NEED_FLOAT_HUGE)

float
nexttowardf (float x, long double y)
{
  uint32_t ux;
  uint32_t e;

  /*
   * We can't do this if y isn't nan as that might raise INEXACT doing
   * long double -> float conversion, and we don't want to do this
   * in long double for machines without long double HW as we won't
   * get any exceptions in that case.
   */
  if (isnan(y))
      return x + (issignaling(y) ? __builtin_nansf("") : (float) y);
  if (isnan(x))
      return x + x;

  if ((long double) x == y)
      return (float) y;
  ux = asuint(x);
  if (x == 0) {
    ux = 1;
    if (signbit(y))
      ux |= 0x80000000;
    x = asfloat(ux);
    force_eval_float(x*x);
    return x;
  } else if ((long double) x < y) {
    if (signbit(x))
      ux--;
    else
      ux++;
  } else {
    if (signbit(x))
      ux++;
    else
      ux--;
  }
  e = ux & 0x7f800000;
  /* raise overflow if ux.value is infinite and x is finite */
  if (e == 0x7f800000)
    return check_oflowf(opt_barrier_float(x+x));
  /* raise underflow if ux.value is subnormal or zero */
  x = asfloat(ux);
  if (e == 0)
      return __math_denormf(x);
  return x;
}

_MATH_ALIAS_f_fl(nexttoward)

#endif /* _NEED_FLOAT_HUGE */
