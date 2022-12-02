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

/* Based on newlib/libm/common/sf_copysign.c in Newlib.  */

#include "amdgcnmach.h"

DEF_VS_MATH_FUNC (v64sf, copysignf, v64sf x, v64sf y)
{
  FUNCTION_INIT (v64sf);

  v64si ix, iy;
  GET_FLOAT_WORD (ix, x, NO_COND);
  GET_FLOAT_WORD (iy, y, NO_COND);
  SET_FLOAT_WORD (x, (ix & 0x7fffffff) | (iy & 0x80000000), NO_COND);
  VECTOR_RETURN (x, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS2 (copysignf, sf, sf)
