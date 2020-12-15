/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include "semihost_syscall.h"
#include "semihost_fdtable.h"

/* Close a file.  */
int
_close (int file)
{
  long res;
  struct fdentry *fd =__get_fdentry (file);
  long data_block[1];

  if (fd == NULL)
    return -1;

  data_block[0] = fd->handle;
  res = syscall_errno (SEMIHOST_close, data_block);

  if (res != 0)
    return res;

  __remove_fdentry (file);
  return 0;
}
