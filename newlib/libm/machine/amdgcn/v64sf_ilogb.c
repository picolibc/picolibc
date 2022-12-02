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
 */

/* Based on newlib/libm/common/sf_ilogb.c in Newlib.  */

#include "amdgcnmach.h"

DEF_VS_MATH_FUNC (v64si, ilogbf, v64sf x)
{
  FUNCTION_INIT(v64si);

  v64si hx, ix;
  GET_FLOAT_WORD (hx, x, NO_COND);
  hx &= 0x7fffffff;
  VECTOR_IF (FLT_UWORD_IS_ZERO (hx), cond)
    VECTOR_RETURN (VECTOR_INIT (-__INT_MAX__), cond);  // FP_ILOGB0
  VECTOR_ENDIF
  VECTOR_IF (FLT_UWORD_IS_SUBNORMAL (hx), cond)
    ix = VECTOR_INIT (-126);
    for (v64si i = (hx << 8);
       !ALL_ZEROES_P (cond & (i > 0));
       i <<= 1)
      VECTOR_COND_MOVE (ix, ix - 1, cond & (i > 0));
    VECTOR_RETURN (ix, cond);
  VECTOR_ELSEIF (~FLT_UWORD_IS_FINITE (hx), cond)
    VECTOR_RETURN (VECTOR_INIT (__INT_MAX__), cond);
  VECTOR_ENDIF

  VECTOR_RETURN ((hx >> 23) - 127, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS (ilogbf, si, sf)
