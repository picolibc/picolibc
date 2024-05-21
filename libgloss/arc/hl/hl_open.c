/*
 * hl_open.c -- provide _open().
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
#include <stdarg.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "hl_toolchain.h"
#include "hl_api.h"

/* Map newlib open flags into Hostlink IO ones.  */
static __always_inline uint32_t
_hl_open_flags_map (int flags)
{
  uint32_t hl_flags = 0;

  hl_flags |= (flags & O_RDONLY) ? 0x0000 : 0;
  hl_flags |= (flags & O_WRONLY) ? 0x0001 : 0;
  hl_flags |= (flags & O_RDWR)   ? 0x0002 : 0;
  hl_flags |= (flags & O_APPEND) ? 0x0008 : 0;
  hl_flags |= (flags & O_CREAT)  ? 0x0100 : 0;
  hl_flags |= (flags & O_TRUNC)  ? 0x0200 : 0;
  hl_flags |= (flags & O_EXCL)   ? 0x0400 : 0;

  return hl_flags;
}

/* Open file on host.  Implements HL_SYSCALL_OPEN.  */
static __always_inline int
_hl_open (const char *path, int flags, mode_t mode)
{
  int32_t fd;
  uint32_t host_errno;
  uint32_t hl_flags = _hl_open_flags_map (flags);
  volatile __uncached char *p;

  p = _hl_message (HL_SYSCALL_OPEN, "sii:ii",
		   path,			/* s */
		   (uint32_t) hl_flags,		/* i */
		   (uint32_t) mode,		/* i */
		   (uint32_t *) &fd,		/* :i */
		   (uint32_t *) &host_errno	/* :i */);

  if (p == NULL)
    {
      errno = ETIMEDOUT;
      fd = -1;
    }
  else if (fd < 0)
    {
      errno = host_errno;
      fd = -1;
    }

  _hl_delete ();

  return fd;
}

int
_open (const char *path, int flags, ...)
{
  va_list ap;
  mode_t mode = 0;

  va_start (ap, flags);

  if (flags & O_CREAT)
    mode = va_arg (ap, mode_t);

  return _hl_open (path, flags, mode);
}
