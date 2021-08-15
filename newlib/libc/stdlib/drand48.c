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

#define _DEFAULT_SOURCE
#include "rand48.h"

double
_drand48_r (struct _rand48 *r)
{
  return _erand48_r(r, r->_seed);
}

#ifndef _REENT_ONLY
double
drand48 (void)
{
  return _drand48_r (&_rand48);
}
#endif /* !_REENT_ONLY */
