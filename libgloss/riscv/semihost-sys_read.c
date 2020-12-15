/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include <errno.h>
#include <sys/types.h>
#include "semihost_syscall.h"
#include "semihost_fdtable.h"

/* Read from a file.  */
ssize_t _read (int file, void *ptr, size_t len)
{
  struct fdentry *fd =__get_fdentry (file);
  long data_block[3];
  long res;

  if (fd == NULL)
    return -1;

  data_block[0] = fd->handle;
  data_block[1] = (long) ptr;
  data_block[2] = len;
  res = syscall_errno (SEMIHOST_read, data_block);
  if (res >= 0)
    {
      ssize_t bytes_read = len - res;
      fd->pos += bytes_read;
      return bytes_read;
    }
  return -1;
}
