/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "semihost_syscall.h"
#include "semihost_fdtable.h"

extern int errno;

/* Set position in a file.  */
off_t
_lseek (int file, off_t offset, int dir)
{
  long data_block[2];
  long flen;
  long res;
  struct fdentry *fd;
  off_t abs_pos;

  fd =__get_fdentry (file);
  if (fd == NULL)
    {
      errno = EBADF;
      return -1;
    }

  if (dir == SEEK_CUR && offset == 0)
    return fd->pos;

  data_block[0] = fd->handle;

  switch (dir)
    {
      case SEEK_SET:
	abs_pos = offset;
	break;
      case SEEK_CUR:
	abs_pos = fd->pos + offset;
	break;
      case SEEK_END:
	data_block[1] = 0;
	flen = syscall_errno (SEMIHOST_flen, data_block);
	if (flen == -1)
	  return -1;
	abs_pos = flen + offset;
	break;
      default:
	errno = EINVAL;
	return -1;
    }

  if (abs_pos < 0)
    {
      errno = EINVAL;
      return -1;
    }

  data_block[1] = abs_pos;
  res = syscall_errno (SEMIHOST_seek, data_block);
  if (res == 0)
    {
      fd->pos = abs_pos;
      return abs_pos;
    }
  return res;
}
