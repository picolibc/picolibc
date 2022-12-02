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
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/* Based on newlib/libm/math/w_lgamma.c in Newlib.  */

#include "amdgcnmach.h"

v64si v64df_finite_aux (v64df x, v64di __mask);
v64df v64df_lgamma_r_aux (v64df x, v64si *signgamp, v64di __mask);

DEF_VD_MATH_FUNC (v64df, lgamma, v64df x)
{
  v64df y = v64df_lgamma_r_aux(x, &(_REENT_V64SI_SIGNGAM(_V64_REENT)), __mask);
  if (ALL_ZEROES_P(v64df_finite_aux(y, __mask)) & !ALL_ZEROES_P(v64df_finite_aux(x, __mask))) {
    /* lgamma(finite) overflow */
    errno = ERANGE;
  }
  return y;
}

DEF_VARIANTS (lgamma, df, df)
