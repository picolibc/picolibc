/*
 * hl_argv.c -- provide _argv().
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

/* Get buffer length for argv[a] using HL_SYSCALL_ARGV.  */
static __always_inline uint32_t
_hl_argvlen (int a)
{
  uint32_t ret = 0;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_ARGV, "i", (uint32_t) a);
  if (p != NULL)
    ret = _hl_get_ptr_len (p);

  _hl_delete ();

  return ret;
}

/* Implements HL_SYSCALL_ARGV.  */
static __always_inline int
_hl_argv (int a, char *arg)
{
  int ret = 0;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_ARGV, "i:s", (uint32_t) a, (char *) arg);
  if (p == NULL)
    ret = -1;

  _hl_delete ();

  return ret;
}

/* See arc-main-helper.c.  */
uint32_t
_argvlen (int a)
{
  return _hl_argvlen (a);
}

/* See arc-main-helper.c.  */
int
_argv (int a, char *arg)
{
  return _hl_argv (a, arg);
}
