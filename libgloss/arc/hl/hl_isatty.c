/*
 * hl_isatty.c -- provide _isatty().
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

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include "hl_toolchain.h"
#include "hl_api.h"

/* Check if fd is a tty on the host.  Implements HL_SYSCALL_ISATTY.  */
static __always_inline int
_hl_isatty (int fd)
{
  int32_t ret;
  uint32_t host_errno;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_ISATTY, "i:ii",
		   (uint32_t) fd,		/* i */
		   (uint32_t *) &ret,		/* :i */
		   (uint32_t *) &host_errno	/* :i */);

  if (p == NULL)
    {
      errno = ETIMEDOUT;
      ret = 0;
    }
  /* isatty() returns 1 if fd is a terminal;
   * otherwise it returns 0 and set errno.
   */
  else if (ret == 0)
    {
      errno = host_errno;
    }
  else
    {
      ret = 1;
    }

  _hl_delete ();

  return ret;
}

int
_isatty (int fd)
{
  return _hl_isatty (fd);
}
