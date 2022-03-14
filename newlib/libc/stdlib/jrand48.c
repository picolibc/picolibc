/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#include "rand48.h"

long
_jrand48_r (struct _rand48 *r,
       unsigned short xseed[3])
{
  int32_t i;
  __dorand48(r, xseed);
  i = (int32_t) ((uint32_t) (xseed[2]) << 16 | (uint32_t) (xseed[1]));
  return i;
}

#ifndef _REENT_ONLY
long
jrand48 (unsigned short xseed[3])
{
  return _jrand48_r (&_rand48, xseed);
}
#endif /* !_REENT_ONLY */
