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
_mrand48_r (struct _rand48 *r)
{
  __dorand48(r, r->_seed);
  return ((long) r->_seed[2] << 16) + (long) r->_seed[1];
}

#ifndef _REENT_ONLY
long
mrand48 (void)
{
  return _mrand48_r (&_rand48);
}
#endif /* !_REENT_ONLY */
