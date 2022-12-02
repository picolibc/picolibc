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

/* Based on newlib/libm/mathfp/e_remainder.c in Newlib.  */

#include "amdgcnmach.h"

v64df v64df_fmod_aux (v64df, v64df, v64di);

#if defined (__has_builtin) && __has_builtin (__builtin_gcn_fabsv)

DEF_VD_MATH_FUNC (v64df, remainder, v64df x, v64df p)
{
  FUNCTION_INIT (v64df);

  v64si hx, lx;
  EXTRACT_WORDS (hx, lx, x);
  v64si hp, lp;
  EXTRACT_WORDS (hp, lp, p);
  v64si sx = hx & 0x80000000;
  hp &= 0x7fffffff;
  hx &= 0x7fffffff;

  /* purge off exception values */
  VECTOR_RETURN ((x * p) / (x * p), ((hp | lp) == 0) | ((hx >= 0x7ff00000)
                                        | /* x not finite */
                                        ((hp >= 0x7ff00000) & /* p is NaN */
                                         (((hp - 0x7ff00000) | lp) != 0))));

  VECTOR_COND_MOVE (x, v64df_fmod_aux (x, p+p, __mask), hp <= 0x7fdfffff); // now x < 2p

  VECTOR_RETURN (0.0 * x, ((hx-hp)|(lx-lp))==0);

  x = __builtin_gcn_fabsv (x);
  p = __builtin_gcn_fabsv (p);

  VECTOR_IF (hp < 0x00200000, cond)
    VECTOR_IF2 (x + x > p, cond2, __builtin_convertvector(cond, v64di))
      VECTOR_COND_MOVE (x, x - p, cond2);
      VECTOR_COND_MOVE (x, x - p, cond2 & (x + x >= p));
    VECTOR_ENDIF
  VECTOR_ELSE (cond)
    v64df p_half = 0.5 * p;
    VECTOR_IF2 (x > p_half, cond2, __builtin_convertvector(cond, v64di))
      VECTOR_COND_MOVE (x, x - p, cond2);
      VECTOR_COND_MOVE (x, x - p, cond2 & (x >= p_half));
    VECTOR_ENDIF
  VECTOR_ENDIF

  GET_HIGH_WORD (hx, x, NO_COND);
  SET_HIGH_WORD (x, hx ^ sx, NO_COND);

  VECTOR_RETURN (x, NO_COND);

  FUNCTION_RETURN;
}

DEF_VARIANTS2 (remainder, df, df)

#endif
