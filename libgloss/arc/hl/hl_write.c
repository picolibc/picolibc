/*
 * hl_write.c -- provide _write().
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
#include <unistd.h>

#include "hl_toolchain.h"
#include "hl_api.h"


/* Write one chunk to the host file.  Implements HL_SYSCALL_WRITE.  */
static ssize_t
_hl_write (int fd, const char *buf, size_t nbyte)
{
  ssize_t ret;
  int32_t n;
  uint32_t host_errno;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_WRITE, "ipi:ii",
		   (uint32_t) fd,		/* i */
		   buf, (uint32_t) nbyte,	/* p */
		   (uint32_t) nbyte,		/* i */
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

ssize_t
_write (int fd, const char *buf, size_t nbyte)
{
  const uint32_t hl_iochunk = _hl_iochunk_size ();
  size_t to_write = nbyte;
  size_t offset = 0;
  ssize_t ret = 0;

  while (to_write > hl_iochunk)
    {
      ret = _hl_write (fd, buf + offset, hl_iochunk);

      if (ret < 0)
	return ret;

      offset += ret;

      if (ret != (ssize_t) hl_iochunk)
	return offset;

      to_write -= hl_iochunk;
    }

  if (to_write)
    {
      ret = _hl_write (fd, buf + offset, to_write);

      if (ret < 0)
	return ret;

      ret += offset;
    }

  return ret;
}
