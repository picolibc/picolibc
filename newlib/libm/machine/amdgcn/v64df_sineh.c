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

/* Based on newlib/libm/mathfp/s_sineh.c in Newlib.  */

#include "amdgcnmach.h"

v64df v64df_exp_aux (v64df, v64di);
v64si v64df_numtest (v64df);
v64si v64df_ispos (v64df);

static const double q[] = { -0.21108770058106271242e+7,
                             0.36162723109421836460e+5,
                            -0.27773523119650701667e+3 };
static const double p[] = { -0.35181283430177117881e+6,
                            -0.11563521196851768270e+5,
                            -0.16375798202630751372e+3,
                            -0.78966127417357099479 };
static const double LNV = 0.6931610107421875000;
static const double INV_V2 = 0.24999308500451499336;
static const double V_OVER2_MINUS1 = 0.13830277879601902638e-4;

#if defined (__has_builtin) && __has_builtin (__builtin_gcn_fabsv)

DEF_VD_MATH_FUNC (v64df, sineh, v64df x, int cosineh)
{
  const double WBAR = 18.55;
  
  FUNCTION_INIT (v64df);

  v64si sgn = VECTOR_INIT (0);
  v64di v_cosineh = VECTOR_INIT (cosineh ? -1L : 0L);

  /* Check for special values. */
  v64si num_type = v64df_numtest (x);
  VECTOR_IF (num_type == NAN, cond)
    errno = EDOM;
    VECTOR_RETURN (x, cond);
  VECTOR_ELSEIF (num_type == INF, cond)
    errno = ERANGE;
    VECTOR_RETURN (VECTOR_MERGE (VECTOR_INIT (z_infinity.d),
				 VECTOR_INIT (-z_infinity.d),
				 v64df_ispos (x)),
		   cond);
  VECTOR_ENDIF

  v64df y = __builtin_gcn_fabsv (x);

  if (!cosineh)
    VECTOR_COND_MOVE (sgn, VECTOR_INIT (-1), x < 0.0);

  v64df res;

  VECTOR_IF (((y > 1.0) & ~v_cosineh) | v_cosineh, cond)
    VECTOR_IF2 (y > BIGX, cond2, cond)
      v64df w = y - LNV;

      /* Check for w > maximum here. */
      VECTOR_IF2 (w > BIGX, cond3, cond2)
	errno = ERANGE;
	VECTOR_RETURN (x, cond3);
      VECTOR_ENDIF

      v64df z = v64df_exp_aux (w, __mask);

      VECTOR_COND_MOVE (res, z * (V_OVER2_MINUS1 + 1.0),
			cond2 & (w > WBAR));
    VECTOR_ELSE2 (cond2, cond)
      v64df z = v64df_exp_aux (y, __mask);
      if (cosineh)
	VECTOR_COND_MOVE (res, (z + 1 / z) * 0.5, cond2);
      else
	VECTOR_COND_MOVE (res, (z - 1 / z) * 0.5, cond2);
    VECTOR_ENDIF

    VECTOR_COND_MOVE (res, -res, sgn);
  VECTOR_ELSE (cond)
    /* Check for y being too small. */
    VECTOR_IF2 (y < z_rooteps, cond2, cond);
      VECTOR_COND_MOVE (res, x, cond2);
    VECTOR_ELSE2 (cond2, cond)
      /* Calculate the Taylor series. */
      v64df f = x * x;
      v64df Q = ((f + q[2]) * f + q[1]) * f + q[0];
      v64df P = ((p[3] * f + p[2]) * f + p[1]) * f + p[0];
      v64df R = f * (P / Q);

      VECTOR_COND_MOVE (res, x + x * R, cond2);
    VECTOR_ENDIF
  VECTOR_ENDIF

  VECTOR_RETURN (res, NO_COND);

  FUNCTION_RETURN;
}

#endif
