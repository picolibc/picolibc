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

v64sf v64sf_logf_aux (v64sf, v64si);

static const float C3 = 1.4426950408889634073599246810019;

DEF_VS_MATH_FUNC (v64sf, log2f, v64sf x)
{
  return v64sf_logf_aux (x, __mask) * C3;
}

DEF_VARIANTS (log2f, sf, sf)
