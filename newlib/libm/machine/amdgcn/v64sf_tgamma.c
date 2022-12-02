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

/* Based on newlib/libm/math/ef_tgamma.c in Newlib. */

#include "amdgcnmach.h"

v64sf v64sf_expf_aux (v64sf x, v64si __mask);
v64sf v64sf_lgammaf_r_aux (v64sf x, v64si *signgamp, v64si __mask);

DEF_VS_MATH_FUNC (v64sf, tgammaf, v64sf x)
{
  v64si signgam_local;
  v64sf y = v64sf_expf_aux(v64sf_lgammaf_r_aux(x, &signgam_local, __mask), __mask);
  VECTOR_COND_MOVE(y, -y, signgam_local < 0);
	return y;
}

DEF_VARIANTS (tgammaf, sf, sf)
