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

/*****************************************************************
 * The following routines are coded directly from the algorithms
 * and coefficients given in "Software Manual for the Elementary
 * Functions" by William J. Cody, Jr. and William Waite, Prentice
 * Hall, 1980.
 *****************************************************************/

/* Based on newlib/libm/mathfp/s_sqrt.c in Newlib.  */

#include "amdgcnmach.h"

v64si v64df_numtest (v64df);
v64si v64df_ispos (v64df);

#if defined (__has_builtin) \
        && __has_builtin (__builtin_gcn_frexpv_mant) \
        && __has_builtin (__builtin_gcn_frexpv_exp) \
        && __has_builtin (__builtin_gcn_ldexpv)

DEF_VD_MATH_FUNC (v64df, sqrt, v64df x)
{
  FUNCTION_INIT (v64df);

  /* Check for special values. */
  v64si num_type = v64df_numtest (x);
  VECTOR_IF (num_type == NAN, cond)
    errno = EDOM;
    VECTOR_RETURN (x, cond);
  VECTOR_ELSEIF (num_type == INF, cond)
    VECTOR_IF2 (v64df_ispos (x), cond2, cond)
      errno = EDOM;
      VECTOR_RETURN (VECTOR_INIT (z_notanum.d), cond2);
    VECTOR_ELSE2 (cond2,cond)
      errno = ERANGE;
      VECTOR_RETURN (VECTOR_INIT (z_infinity.d), cond);
    VECTOR_ENDIF
  VECTOR_ENDIF

  /* Initial checks are performed here. */
  VECTOR_IF (x == 0.0, cond)
    VECTOR_RETURN (VECTOR_INIT (0.0), cond);
  VECTOR_ENDIF
  VECTOR_IF (x < 0.0, cond)
    errno = EDOM;
    VECTOR_RETURN (VECTOR_INIT (z_notanum.d), cond);
  VECTOR_ENDIF

  /* Find the exponent and mantissa for the form x = f * 2^exp. */
  v64df f = __builtin_gcn_frexpv_mant (x);
  v64si exp = __builtin_gcn_frexpv_exp (x);
  v64si odd = (exp & 1) != 0;

  /* Get the initial approximation. */
  v64df y = 0.41731 + 0.59016 * f;

  f *= 0.5f;
  /* Calculate the remaining iterations. */
  y = y * 0.5f + f / y;
  y = y * 0.5f + f / y;
  y = y * 0.5f + f / y;

  /* Calculate the final value. */
  VECTOR_COND_MOVE (y, y * __SQRT_HALF, odd);
  VECTOR_COND_MOVE (exp, exp + 1, odd);
  exp >>= 1;
  y = __builtin_gcn_ldexpv (y, exp);

  VECTOR_RETURN (y, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS (sqrt, df, df)

#endif
