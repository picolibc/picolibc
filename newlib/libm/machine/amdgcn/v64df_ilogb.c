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

/* Based on newlib/libm/common/s_ilogb.c in Newlib.  */

#include "amdgcnmach.h"

DEF_VD_MATH_PRED (v64si, ilogb, v64df x)
{
  FUNCTION_INIT(v64si);
  v64si hx, lx, ix;
  EXTRACT_WORDS (hx, lx, x);
  hx &= 0x7fffffff;
  VECTOR_IF (hx < 0x00100000, cond)
    VECTOR_RETURN (VECTOR_INIT (-__INT_MAX__), cond & ((hx | lx) == 0));  // FP_ILOGB0
    VECTOR_IF2 (hx == 0, cond2, cond)
      ix = VECTOR_INIT (-1043);
      for (v64si i = lx;
            !ALL_ZEROES_P (cond2 & (i > 0));
            i <<= 1)
        VECTOR_COND_MOVE (ix, ix - 1, cond2 & (i > 0));
    VECTOR_ELSE2 (cond2, cond)
      ix = VECTOR_INIT (-1022);
      for (v64si i = (hx << 11);
            !ALL_ZEROES_P (cond2 & (i > 0));
            i <<= 1)
        VECTOR_COND_MOVE (ix, ix - 1, cond2 & (i > 0));
    VECTOR_ENDIF
    VECTOR_RETURN (ix, cond);
  VECTOR_ENDIF
  VECTOR_RETURN ((hx >> 20) - 1023, hx < 0x7ff00000);
  VECTOR_RETURN (VECTOR_INIT (__INT_MAX__), NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS (ilogb, si, df)
