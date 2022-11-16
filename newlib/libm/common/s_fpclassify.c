/* Copyright (C) 2002, 2007 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#define _ADD_D_TO_DOUBLE_FUNCS

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

int
__fpclassify64 (__float64 x)
{
  __uint32_t msw, lsw;

  EXTRACT_WORDS(msw,lsw,x);

  msw &= 0x7fffffff;
  if (msw == 0x00000000 && lsw == 0x00000000)
    return FP_ZERO;
  else if (msw <= 0x000fffff)
    /* zero is already handled above */
    return FP_SUBNORMAL;
  else if (msw <= 0x7fefffff)
    return FP_NORMAL;
  else if (msw == 0x7ff00000 && lsw == 0x00000000)
    return FP_INFINITE;
  else
    return FP_NAN;
}

_MATH_ALIAS_i_d(__fpclassify)

#endif /* _NEED_FLOAT64 */
