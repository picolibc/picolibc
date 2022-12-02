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

/* Based on newlib/libm/mathfp/ef_scalb.c in Newlib.  */

#include "amdgcnmach.h"

v64si v64sf_isnanf (v64sf);
v64si v64sf_finitef (v64sf);
v64sf v64sf_rintf_aux (v64sf, v64si);
v64sf v64sf_scalbnf_aux (v64sf, v64si, v64si);

DEF_VS_MATH_FUNC (v64sf, scalbf, v64sf x, v64sf fn)
{
  FUNCTION_INIT (v64sf);

  VECTOR_IF (v64sf_isnanf(x) | v64sf_isnanf(fn), cond)
    VECTOR_RETURN (x * fn, cond);
  VECTOR_ENDIF
  VECTOR_IF (~v64sf_finitef (fn), cond)
    VECTOR_IF2 (fn > 0.0f, cond2, cond)
      VECTOR_RETURN (x * fn, cond2);
    VECTOR_ELSE2 (cond2, cond)
      VECTOR_RETURN (x / (-fn), cond2);
    VECTOR_ENDIF
  VECTOR_ENDIF
  VECTOR_IF (v64sf_rintf_aux (fn, __mask) != fn, cond)
    VECTOR_RETURN ((fn-fn)/(fn-fn), cond);
  VECTOR_ENDIF
#if INT_MAX > 65000
  VECTOR_IF (fn > 65000.0f, cond)
    VECTOR_RETURN (v64sf_scalbnf_aux (x, VECTOR_INIT (65000), __mask), cond);
  VECTOR_ENDIF
  VECTOR_IF (-fn > 65000.0f, cond)
    VECTOR_RETURN (v64sf_scalbnf_aux (x, VECTOR_INIT (-65000), __mask), cond);
  VECTOR_ENDIF
#else
  VECTOR_IF (fn > 32000.0f, cond)
    VECTOR_RETURN (v64sf_scalbnf_aux (x, VECTOR_INIT (32000), __mask), cond);
  VECTOR_ENDIF
  VECTOR_IF (-fn > 32000.0f, cond)
    VECTOR_RETURN (v64sf_scalbnf_aux (x, VECTOR_INIT (-32000), __mask), cond);
  VECTOR_ENDIF
#endif
  VECTOR_RETURN (v64sf_scalbnf_aux (x, __builtin_convertvector (fn, v64si), __mask),
		 NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS2 (scalbf, sf, sf)
