/* Copyright (C) 2015 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <complex.h>
#include "../common/fdlibm.h"

long double
cimagl (long double complex z)
{
  long_double_complex w = { .z = z };

  return IMAG_PART (w);
}
