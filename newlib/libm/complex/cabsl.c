/* Copyright (C) 2015 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <complex.h>
#include <math.h>

long double
cabsl (long double complex z)
{
#ifdef _LDBL_EQ_DBL
  return cabs (z);
#else
  return hypotl (creall (z), cimagl (z));
#endif
}
