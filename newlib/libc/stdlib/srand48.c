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

void
_srand48_r (struct _rand48 *r,
       long seed)
{
  r->_seed[0] = _RAND48_SEED_0;
  r->_seed[1] = (unsigned short) seed;
  r->_seed[2] = (unsigned short) ((unsigned long)seed >> 16);
  r->_mult[0] = _RAND48_MULT_0;
  r->_mult[1] = _RAND48_MULT_1;
  r->_mult[2] = _RAND48_MULT_2;
  r->_add = _RAND48_ADD;
}

#ifndef _REENT_ONLY
void
srand48 (long seed)
{
  _srand48_r (&_rand48, seed);
}
#endif /* !_REENT_ONLY */
