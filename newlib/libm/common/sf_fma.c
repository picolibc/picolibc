/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

#if !_HAVE_FAST_FMAF

float fmaf(float x, float y, float z)
{
  /*
   * Better to just get a double-rounded answer than invoke double
   * precision operations and get exceptions messed up
   */
  return x * y + z;
}

_MATH_ALIAS_f_fff(fma)

#endif
