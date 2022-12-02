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

/* Based on newlib/libm/mathfp/sf_asinh.c in Newlib.  */

#include "amdgcnmach.h"

v64sf v64sf_logf_aux (v64sf, v64si);
v64sf v64sf_log1pf_aux (v64sf, v64si);
v64sf v64sf_sqrtf_aux (v64sf, v64si);

#if defined (__has_builtin) && __has_builtin (__builtin_gcn_fabsvf)

DEF_VS_MATH_FUNC (v64sf, asinhf, v64sf x)
{
  static const float one =  1.0000000000e+00; /* 0x3F800000 */
  static const float ln2 =  6.9314718246e-01; /* 0x3f317218 */
  static const float huge = 1.0000000000e+30;

  FUNCTION_INIT (v64sf);

  v64sf w;
  v64si hx;
  GET_FLOAT_WORD (hx, x, NO_COND);
  v64si ix = hx & 0x7fffffff;

  VECTOR_IF (ix >=0x7f800000, cond)	/* x is inf or NaN */
    VECTOR_RETURN (x + x, cond);
  VECTOR_ENDIF
  VECTOR_IF (ix < 0x31800000, cond)	/* |x|<2**-28 */
    VECTOR_IF2 (huge+x > one, cond2, cond) /* return x inexact except 0 */
      VECTOR_RETURN (x, cond);
    VECTOR_ENDIF
  VECTOR_ENDIF
  VECTOR_IF (ix > 0x4d800000, cond) 	/* x > 2**28 */
    VECTOR_COND_MOVE (w, v64sf_logf_aux (__builtin_gcn_fabsvf (x), __mask) +
		      ln2,
		      cond);
  VECTOR_ELSEIF (ix > 0x40000000, cond)	/* 2**28 > |x| > 2.0 */
    v64sf t = __builtin_gcn_fabsvf (x);
    VECTOR_COND_MOVE (w, v64sf_logf_aux (2.0f * t + 1.0f / (v64sf_sqrtf_aux (x*x + 1.0f, __mask) + t), __mask),
		      cond);
  VECTOR_ELSE (cond)			/* 2.0 > |x| > 2**-28 */
    v64sf t = x * x;
    VECTOR_COND_MOVE (w, v64sf_log1pf_aux (__builtin_gcn_fabsvf (x) + t / (1.0f + v64sf_sqrtf_aux (1.0f + t, __mask)), __mask),
		      cond);
  VECTOR_ENDIF

  VECTOR_IF (hx > 0, cond)
    VECTOR_RETURN (w, cond);
  VECTOR_ELSE (cond)
    VECTOR_RETURN (-w, cond);
  VECTOR_ENDIF

  FUNCTION_RETURN;
}

DEF_VARIANTS (asinhf, sf, sf)

#endif
