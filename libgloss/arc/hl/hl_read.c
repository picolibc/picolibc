/*
 * hl_read.c -- provide _read().
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


/* Read one chunk.  Implements HL_SYSCALL_READ.  */
static ssize_t
_hl_read (int fd, void *buf, size_t count)
{
  ssize_t ret;
  int32_t hl_n;
  uint32_t host_errno;
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_READ, "ii:i",
		   (uint32_t) fd,      /* i */
		   (uint32_t) count,   /* i */
		   (uint32_t *) &hl_n  /* :i */);

  if (p == NULL)
    {
      errno = ETIMEDOUT;
      ret = -1;
    }
  else if (hl_n < 0)
    {
      p = _hl_unpack_int (p, &host_errno);
      errno = p == NULL ? EIO : host_errno;
      ret = -1;
    }
  else
    {
      uint32_t n;

      p = _hl_unpack_ptr (p, buf, &n);
      ret = n;

      if (p == NULL || n != (uint32_t) hl_n)
	{
	  errno = EIO;
	  ret = -1;
	}
    }

  _hl_delete ();

  return ret;
}

ssize_t
_read (int fd, void *buf, size_t count)
{
  const uint32_t hl_iochunk = _hl_iochunk_size ();
  size_t to_read = count;
  size_t offset = 0;
  ssize_t ret = 0;

  while (to_read > hl_iochunk)
    {
      ret = _hl_read (fd, (char *) buf + offset, hl_iochunk);

      if (ret < 0)
	return ret;

      offset += ret;

      if (ret != (ssize_t) hl_iochunk)
	return offset;

      to_read -= hl_iochunk;
    }

  if (to_read)
    {
      ret = _hl_read (fd, (char *) buf + offset, to_read);

      if (ret < 0)
	return ret;

      ret += offset;
    }

  return ret;
}
