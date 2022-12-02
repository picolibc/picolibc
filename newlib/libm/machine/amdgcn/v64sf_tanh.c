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

/* Based on newlib/libm/mathfp/sf_tanh.c in Newlib.  */

#include "amdgcnmach.h"

v64sf v64sf_expf_aux (v64sf, v64si);

static const float LN3_OVER2 = 0.54930614433405484570;
static const float p[] = { -0.16134119023996228053e+4,
                           -0.99225929672236083313e+2,
                           -0.96437492777225469787 };
static const float q[] = { 0.48402357071988688686e+4,
                           0.22337720718962312926e+4,
                           0.11274474380534949335e+3 }; 

#if defined (__has_builtin) && __has_builtin (__builtin_gcn_fabsvf)

DEF_VS_MATH_FUNC (v64sf, tanhf, v64sf x)
{
  FUNCTION_INIT (v64sf);

  v64sf f = __builtin_gcn_fabsvf (x);
  v64sf res;

  /* Check if the input is too big. */
  VECTOR_IF (f > (float) BIGX, cond)
    VECTOR_COND_MOVE (res, VECTOR_INIT (1.0f), cond);

  VECTOR_ELSEIF (f > LN3_OVER2, cond)
    VECTOR_COND_MOVE (res, 1.0f - 2.0f / (v64sf_expf_aux (2.0f * f, __mask) + 1.0f),
		      cond);

  /* Check if the input is too small. */
  VECTOR_ELSEIF (f < z_rooteps_f, cond)
    VECTOR_COND_MOVE (res, f, cond);

  /* Calculate the Taylor series. */
  VECTOR_ELSE (cond)
    v64sf g = f * f;

    v64sf P = (p[2] * g + p[1]) * g + p[0];
    v64sf Q = ((g + q[2]) * g + q[1]) * g + q[0];
    v64sf R = g * (P / Q);

    VECTOR_COND_MOVE (res, f + f * R, cond);
  VECTOR_ENDIF

  VECTOR_COND_MOVE (res, -res, x < 0.0f);

  VECTOR_RETURN (res, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS (tanhf, sf, sf)

#endif
