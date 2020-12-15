/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include "semihost_syscall.h"
#include <sys/stat.h>
#include "semihost_fdtable.h"

/* Used by _fstat and _stat to fill in some common details.  */

int
__stat_common (int file, struct stat *st)
{
  int flen;
  struct fdentry *fd =__get_fdentry (file);
  long data_block[1];

  if (fd == NULL)
    return -1;

  data_block[0] = fd->handle;

  /* Assume character device and default block size of 4096.  */
  st->st_mode |= S_IFCHR;
  st->st_blksize = 4096;

  /* Attempt to get length of file.  */
  flen = syscall_errno (SEMIHOST_flen, data_block);
  if (flen == -1)
    return -1;

  st->st_size = flen;

  return 0;
}
