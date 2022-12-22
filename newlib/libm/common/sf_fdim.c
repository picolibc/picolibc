/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

float fdimf(float x, float y)
{
  if (isnanf(x) || isnanf(y)) return(x+y);

  float z = x > y ? x - y : 0.0f;
  if (!isinf(x) && !isinf(y))
    z = check_oflowf(z);
  return z;
}

_MATH_ALIAS_f_ff(fdim)
