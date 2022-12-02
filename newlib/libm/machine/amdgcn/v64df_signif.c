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

/* Based on newlib/libm/mathfp/s_signif.c in Newlib.  */

#include "amdgcnmach.h"

v64df v64df_scalb_aux (v64df x, v64df fn, v64di);
v64si v64df_ilogb_aux (v64df x, v64si);

DEF_VD_MATH_FUNC (v64df, significand, v64df x)
{
  v64si si_mask = __builtin_convertvector (__mask, v64si);
  return v64df_scalb_aux (x, -__builtin_convertvector (v64df_ilogb_aux (x, si_mask), v64df), __mask);
}

DEF_VARIANTS (significand, df, df)
