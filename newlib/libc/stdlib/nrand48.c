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
_nrand48_r (struct _rand48 *r,
       unsigned short xseed[3])
{
  __dorand48 (r, xseed);
  return (long)((unsigned long) xseed[2] << 15) +
    ((unsigned long) xseed[1] >> 1);
}

long
nrand48 (unsigned short xseed[3])
{
  return _nrand48_r (&_rand48, xseed);
}
