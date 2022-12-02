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

#include "amdgcnmach.h"

v64df v64df_log_aux (v64df, v64di);

static const double C3 = 1.4426950408889634073599246810019;

DEF_VD_MATH_FUNC (v64df, log2, v64df x)
{
  return v64df_log_aux (x, __mask) * C3;
}

DEF_VARIANTS (log2, df, df)
