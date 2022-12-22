/* Copyright (C) 2002, 2007 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

int
__fpclassifyl (long double x)
{
  u_int32_t hx,lx,esx;

  GET_LDOUBLE_WORDS(esx,hx,lx,x);

  esx &= 0x7fff;
  if (esx == 0 && hx == 0 && lx == 0)
    return FP_ZERO;
  else if (esx == 0)
    /* zero is already handled above */
    return FP_SUBNORMAL;
  else if (esx < 0x7fff)
    return FP_NORMAL;
  else if (hx == LDBL_NBIT && lx == 0)
    return FP_INFINITE;
  else
    return FP_NAN;
}

