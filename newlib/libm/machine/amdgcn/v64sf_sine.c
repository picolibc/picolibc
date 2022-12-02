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

/* Based on newlib/libm/mathfp/sf_sine.c in Newlib.  */

#include "amdgcnmach.h"

v64si v64sf_numtestf (v64sf);

static const float HALF_PI = 1.570796326;
static const float ONE_OVER_PI = 0.318309886;
static const float r[] = { -0.1666665668,
                            0.8333025139e-02,
                           -0.1980741872e-03,
                            0.2601903036e-5 };

#if defined (__has_builtin) && __has_builtin (__builtin_gcn_fabsvf)

DEF_VS_MATH_FUNC (v64sf, sinef, v64sf x, int cosine)
{
  const float YMAX = 210828714.0;

  FUNCTION_INIT (v64sf);

  v64si num_type = v64sf_numtestf (x);
  VECTOR_IF (num_type == NAN, cond)
    errno = EDOM;
    VECTOR_RETURN (x, cond);
  VECTOR_ELSEIF (num_type == INF, cond)
    errno = EDOM;
    VECTOR_RETURN (VECTOR_INIT (z_notanum_f.f), cond);
  VECTOR_ENDIF

  /* Use sin and cos properties to ease computations. */
  v64si sgn;
  v64sf y;

  if (cosine)
    {
      sgn = VECTOR_INIT (0);
      y = __builtin_gcn_fabsvf (x) + HALF_PI;
    }
  else
    {
      sgn = x < 0.0f;
      y = VECTOR_MERGE (-x, x, x < 0.0f);
    }

  /* Check for values of y that will overflow here. */
  VECTOR_IF (y > YMAX, cond)
    errno = ERANGE;
    VECTOR_RETURN (x, cond);
  VECTOR_ENDIF

  /* Calculate the exponent. */
  v64si Nneg = __builtin_convertvector (y * ONE_OVER_PI - 0.5f, v64si);
  v64si Npos = __builtin_convertvector (y * ONE_OVER_PI + 0.5f, v64si);
  v64si N = VECTOR_MERGE (Nneg, Npos, y < 0.0f);
  v64sf XN = __builtin_convertvector (N, v64sf);

  VECTOR_COND_MOVE (sgn, ~sgn, (N & 1) != 0);

  if (cosine)
    XN -= 0.5;

  y = __builtin_gcn_fabsvf (x) - XN * (float) __PI;

  v64sf res;

  VECTOR_IF ((-z_rooteps_f < y) & (y < z_rooteps_f), cond)
    VECTOR_COND_MOVE (res, y, cond);
  VECTOR_ELSE (cond)
    v64sf g = y * y;

    /* Calculate the Taylor series. */
    v64sf R = (((r[3] * g + r[2]) * g + r[1]) * g + r[0]) * g;

    /* Finally, compute the result. */
    VECTOR_COND_MOVE (res, y + y * R, cond);
  VECTOR_ENDIF
 
  VECTOR_COND_MOVE (res, -res, sgn);

  VECTOR_RETURN (res, NO_COND);

  FUNCTION_RETURN;
}

#endif
