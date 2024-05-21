/*
 * hl_lseek.c -- provide _lseek().
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
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#include "hl_toolchain.h"
#include "hl_api.h"


/* Implements HL_SYSCALL_LSEEK.  */
static __always_inline off_t
_hl_lseek (int fd, off_t offset, int whence)
{
  ssize_t ret;
  int32_t n;
  uint32_t host_errno;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_LSEEK, "iii:ii",
		   (uint32_t) fd,		/* i */
		   (uint32_t) offset,		/* i */
		   (int32_t) whence,		/* i */
		   (uint32_t *) &n,		/* :i */
		   (uint32_t *) &host_errno	/* :i */);

  if (p == NULL)
    {
      errno = ETIMEDOUT;
      ret = -1;
    }
  else if (n < 0)
    {
      errno = host_errno;
      ret = -1;
    }
  else
    {
      ret = n;
    }

  _hl_delete ();

  return ret;
}

off_t
_lseek (int fd, off_t offset, int whence)
{
  return _hl_lseek (fd, offset, whence);
}
