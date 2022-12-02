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

/* Based on newlib/libm/common/s_finite.c in Newlib.  */

#include "amdgcnmach.h"

DEF_VD_MATH_PRED (v64si, finite, v64df x)
{
  FUNCTION_INIT (v64si);
  v64si hx;
  GET_HIGH_WORD (hx, x, NO_COND);
  return (((hx & 0x7fffffff) - 0x7ff00000) >> 31) != 0;
}

DEF_VARIANTS (finite, si, df)
