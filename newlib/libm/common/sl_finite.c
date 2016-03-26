/* Copyright (C) 2015 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/* finitel(x) returns 1 is x is finite, else 0; */

#include <math.h>

int
finitel (long double x)
{
#ifdef _LDBL_EQ_DBL
  return finite (x);
#else
  /* Let the compiler do this for us.
     Note - we do not know how many bits there are in a long double.
     Some architectures for example have an 80-bit long double whereas
     others use 128-bits.  We use macros and comiler builtin functions
     to avoid specific knowledge of the long double format.  */
  return __builtin_isfinite (x);
#endif
}

