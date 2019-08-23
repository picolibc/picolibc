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
_lcong48_r (struct _rand48 *r,
       unsigned short p[7])
{
  r->_seed[0] = p[0];
  r->_seed[1] = p[1];
  r->_seed[2] = p[2];
  r->_mult[0] = p[3];
  r->_mult[1] = p[4];
  r->_mult[2] = p[5];
  r->_add = p[6];
}

#ifndef _REENT_ONLY
void
lcong48 (unsigned short p[7])
{
  _lcong48_r (&_rand48, p);
}
#endif /* !_REENT_ONLY */
