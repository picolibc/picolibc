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

unsigned short *
_seed48_r (struct _rand48 *r,
       unsigned short xseed[3])
{
  static NEWLIB_THREAD_LOCAL unsigned short sseed[3];

  sseed[0] = r->_seed[0];
  sseed[1] = r->_seed[1];
  sseed[2] = r->_seed[2];
  r->_seed[0] = xseed[0];
  r->_seed[1] = xseed[1];
  r->_seed[2] = xseed[2];
  r->_mult[0] = _RAND48_MULT_0;
  r->_mult[1] = _RAND48_MULT_1;
  r->_mult[2] = _RAND48_MULT_2;
  r->_add = _RAND48_ADD;
  return sseed;
}

#ifndef _REENT_ONLY
unsigned short *
seed48 (unsigned short xseed[3])
{
  return _seed48_r (&_rand48, xseed);
}
#endif /* !_REENT_ONLY */
