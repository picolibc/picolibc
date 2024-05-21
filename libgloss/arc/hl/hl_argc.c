/*
 * hl_argc.c -- provide _argc().
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#include <stddef.h>

#include "hl_toolchain.h"
#include "hl_api.h"

/* Implements HL_SYSCALL_ARGC.  */
static __always_inline int
_hl_argc (void)
{
  uint32_t ret;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_ARGC, ":i", (uint32_t *) &ret);

  if (p == NULL)
    ret = 0;

  _hl_delete ();

  return ret;
}

/* See arc-main-helper.c.  */
int
_argc (void)
{
  return _hl_argc ();
}
