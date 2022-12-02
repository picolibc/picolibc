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
 * Copyright (c) 1994-2009  Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the BSD License.   This program is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
 * including the implied warranties of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  A copy of this license is available at
 * http://www.opensource.org/licenses. Any Red Hat trademarks that are
 * incorporated in the source code or documentation are not subject to
 * the BSD License and may only be used or replicated with the express
 * permission of Red Hat, Inc.
 */

/* Based on newlib/libm/mathfp/s_numtest.c in Newlib.  */

#include "amdgcnmach.h"

v64si
v64df_numtest (v64df x)
{
  // Explicitly create mask for internal function.
  v64si __mask = VECTOR_INIT (-1);
  FUNCTION_INIT (v64si);

  v64si hx, lx;
  EXTRACT_WORDS (hx, lx, x);
  v64si exp = (hx & 0x7ff00000) >> 20;

  /* Check for a zero input. */
  VECTOR_RETURN (VECTOR_INIT (0), x == 0.0);

  /* Check for not a number or infinity. */
  VECTOR_IF (exp == 0x7ff, cond)
    VECTOR_RETURN (VECTOR_MERGE (VECTOR_INIT (NAN),
				 VECTOR_INIT (INF),
				 ((hx & 0xf0000) != 0) | (lx != 0)),
		   cond);
  /* Otherwise it's a finite value. */
  VECTOR_ELSE (cond)
    VECTOR_RETURN (VECTOR_INIT (NUM), cond);
  VECTOR_ENDIF

  FUNCTION_RETURN;
}
