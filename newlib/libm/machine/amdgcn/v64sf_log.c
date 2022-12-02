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

/* Based on newlib/libm/mathfp/sf_logarithm.c in Newlib.  */

#include "amdgcnmach.h"

v64si v64sf_finitef_aux (v64sf, v64si);
v64si v64sf_isnanf_aux (v64sf, v64si);

static const float a[] = { -0.64124943423745581147e+02,
                           0.16383943563021534222e+02,
                           -0.78956112887481257267 };
static const float b[] = { -0.76949932108494879777e+03,
                           0.31203222091924532844e+03,
                           -0.35667977739034646171e+02 };
static const float C1 = 0.693145752;
static const float C2 = 1.428606820e-06;

#if defined (__has_builtin) \
        && __has_builtin (__builtin_gcn_frexpvf_mant) \
        && __has_builtin (__builtin_gcn_frexpvf_exp)

DEF_VS_MATH_FUNC (v64sf, logf, v64sf x)
{
  FUNCTION_INIT (v64sf);

  /* Check for domain/range errors here. */
  VECTOR_IF (x == 0.0f, cond)
    errno = ERANGE;
    VECTOR_RETURN (VECTOR_INIT (-z_infinity_f.f), cond);
  VECTOR_ELSEIF (x < 0.0f, cond)
    errno = EDOM;
    VECTOR_RETURN (VECTOR_INIT (z_notanum_f.f), cond);
  VECTOR_ELSEIF (~v64sf_finitef_aux (x, __mask), cond)
    VECTOR_RETURN (VECTOR_MERGE (VECTOR_INIT (z_notanum_f.f),
                                 VECTOR_INIT (z_infinity_f.f),
                                 v64sf_isnanf_aux (x, __mask)),
                   cond);
  VECTOR_ENDIF

  /* Get the exponent and mantissa where x = f * 2^N. */
  v64sf f = __builtin_gcn_frexpvf_mant (x);
  v64si N = __builtin_gcn_frexpvf_exp (x);

  v64sf z = f - 0.5f;

  VECTOR_IF (f > (float) __SQRT_HALF, cond)
    VECTOR_COND_MOVE (z, (z - 0.5f) / (f * 0.5f + 0.5f), cond);
  VECTOR_ELSE (cond)
    VECTOR_COND_MOVE (N, N - 1, cond);
    VECTOR_COND_MOVE (z, z / (z * 0.5f + 0.5f), cond);
  VECTOR_ENDIF

  v64sf w = z * z;

  /* Use Newton's method with 4 terms. */
  z += z * w * ((a[2] * w + a[1]) * w + a[0]) / (((w + b[2]) * w + b[1]) * w + b[0]);

  v64sf Nf = __builtin_convertvector(N, v64sf);
  VECTOR_COND_MOVE (z, (Nf * C2 + z) + Nf * C1, N != 0);

  VECTOR_RETURN (z, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS (logf, sf, sf)

DEF_VS_MATH_FUNC (v64sf, log1pf, v64sf x)
{
  /* TODO: Implement algorithm with better precision.  */
  return v64sf_logf_aux (1 + x, __mask);
}

DEF_VARIANTS (log1pf, sf, sf)

#endif
