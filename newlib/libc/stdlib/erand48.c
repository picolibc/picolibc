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

double
_erand48_r (struct _rand48 *r,
       unsigned short xseed[3])
{
  __dorand48(r, xseed);
  return ldexp((double) xseed[0], -48) +
    ldexp((double) xseed[1], -32) +
    ldexp((double) xseed[2], -16);
}

double
erand48 (unsigned short xseed[3])
{
  return _erand48_r (&_rand48, xseed);
}
