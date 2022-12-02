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

/* Based on newlib/libm/math/e_tgamma.c in Newlib. */

#include "amdgcnmach.h"

v64df v64df_exp_aux (v64df x, v64di __mask);
v64df v64df_lgamma_r_aux (v64df x, v64si *signgamp, v64di __mask);

DEF_VD_MATH_FUNC (v64df, tgamma, v64df x)
{
  v64si signgam_local;
  v64df y = v64df_exp_aux(v64df_lgamma_r_aux(x, &signgam_local, __mask), __mask);
  VECTOR_COND_MOVE(y, -y, signgam_local < 0);
	return y;
}

DEF_VARIANTS (tgamma, df, df)
