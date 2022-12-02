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

/* Based on newlib/libm/mathfp/ef_atanh.c in Newlib.  */

#include "amdgcnmach.h"

v64sf v64sf_log1pf_aux (v64sf, v64si);

DEF_VS_MATH_FUNC (v64sf, atanhf, v64sf x)
{
  static const float zero = 0.0;
  static const float one = 1.0, huge = 1e30;

  FUNCTION_INIT (v64sf);

  v64sf t;
  v64si hx;
  GET_FLOAT_WORD (hx, x, NO_COND);
  v64si ix = hx & 0x7fffffff;

  VECTOR_IF (ix > 0x3f800000, cond)		// |x|>1
    VECTOR_RETURN ((x - x)/(x - x), cond);
  VECTOR_ENDIF

  VECTOR_IF (ix == 0x3f800000, cond)
    VECTOR_RETURN (x / zero, cond);
  VECTOR_ENDIF

  VECTOR_IF ((ix < 0x31800000) & ((huge + x) > zero), cond)	// x<2**-28
    VECTOR_RETURN (x, cond);
  VECTOR_ENDIF

  SET_FLOAT_WORD (x, ix, NO_COND);

  VECTOR_IF (ix < 0x3f000000, cond)		// x < 0.5 */
    v64sf t2 = x + x;
    VECTOR_COND_MOVE (t, 0.5f * v64sf_log1pf_aux (t2 + t2 * x / (one - x), __mask), cond);
  VECTOR_ELSE (cond)
    VECTOR_COND_MOVE (t, 0.5f * v64sf_log1pf_aux ((x + x) / (one - x), __mask), cond);
  VECTOR_ENDIF

  VECTOR_IF (hx >= 0, cond)
    VECTOR_RETURN (t, cond);
  VECTOR_ELSE (cond)
    VECTOR_RETURN (-t, cond);
  VECTOR_ENDIF

  FUNCTION_RETURN;
}

DEF_VARIANTS (atanhf, sf, sf)
