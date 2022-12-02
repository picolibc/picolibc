/*
 * Copyright 2023 Siemens
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/*
 * Copyright (c) 1994-2009  Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the BSD License.   This program is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
 * including the implied warranties of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  A copy of this license is available at
 * http://www.opensource.org/licenses. Any Red Hat trademarks that are
 * incorporated in the source code or documentation are not subject to
 * the BSD License and may only be used or replicated with the express
 * permission of Red Hat, Inc.
 */

/******************************************************************
 * The following routines are coded directly from the algorithms
 * and coefficients given in "Software Manual for the Elementary
 * Functions" by William J. Cody, Jr. and William Waite, Prentice
 * Hall, 1980.
 ******************************************************************/

/* Based on newlib/libm/mathfp/s_logarithm.c in Newlib.  */

#include "amdgcnmach.h"

v64si v64df_finite (v64df);
v64si v64df_isnan (v64df);

static const double a[] = { -0.64124943423745581147e+02,
                            0.16383943563021534222e+02,
                            -0.78956112887481257267 };
static const double b[] = { -0.76949932108494879777e+03,
                            0.31203222091924532844e+03,
                            -0.35667977739034646171e+02 };
static const double C1 =  22713.0 / 32768.0;
static const double C2 =  1.428606820309417232e-06;

#if defined (__has_builtin) \
        && __has_builtin (__builtin_gcn_frexpv_mant) \
        && __has_builtin (__builtin_gcn_frexpv_exp) \

DEF_VD_MATH_FUNC (v64df, log, v64df x)
{
  FUNCTION_INIT (v64df);

  /* Check for domain/range errors here. */
  VECTOR_IF (x == 0.0, cond)
    errno = ERANGE;
    VECTOR_RETURN (VECTOR_INIT (-z_infinity.d), cond);
  VECTOR_ELSEIF (x < 0.0, cond)
    errno = EDOM;
    VECTOR_RETURN (VECTOR_INIT (z_notanum.d), cond);
  VECTOR_ELSEIF (__builtin_convertvector (~v64df_finite (x), v64di), cond)
    VECTOR_RETURN (VECTOR_MERGE (VECTOR_INIT (z_notanum.d),
                                 VECTOR_INIT (z_infinity.d),
                                 v64df_isnan (x)),
                   cond);
  VECTOR_ENDIF

  /* Get the exponent and mantissa where x = f * 2^N. */
  v64df f = __builtin_gcn_frexpv_mant (x);
  v64si N = __builtin_gcn_frexpv_exp (x);

  v64df z = f - 0.5;

  VECTOR_IF (f > __SQRT_HALF, cond)
    VECTOR_COND_MOVE (z, (z - 0.5) / (f * 0.5 + 0.5), cond);
  VECTOR_ELSE (cond)
    VECTOR_COND_MOVE (N, N - 1, cond);
    VECTOR_COND_MOVE (z, z / (z * 0.5 + 0.5), cond);
  VECTOR_ENDIF

  v64df w = z * z;

  /* Use Newton's method with 4 terms. */
  z += z * w * ((a[2] * w + a[1]) * w + a[0]) / (((w + b[2]) * w + b[1]) * w + b[0]);

  v64df Nf = __builtin_convertvector (N, v64df);
  VECTOR_COND_MOVE (z, (Nf * C2 + z) + Nf * C1, N != 0);

  VECTOR_RETURN (z, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS (log, df, df)

DEF_VD_MATH_FUNC (v64df, log1p, v64df x)
{
  /* TODO: Implement algorithm with better precision.  */
  return v64df_log_aux (1 + x, __mask);
}

DEF_VARIANTS (log1p, df, df)

#endif
