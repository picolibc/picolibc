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
_lrand48_r (struct _rand48 *r)
{
  __dorand48(r, r->_seed);
  return (long)((unsigned long) r->_seed[2] << 15) +
    ((unsigned long) r->_seed[1] >> 1);
}

long
lrand48 (void)
{
  return _lrand48_r (&_rand48);
}
