/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include <string.h>
#include <fcntl.h>
#include "semihost_stat.h"

/* Status of a file (by name).  */

int
_stat (const char *name, struct stat *st)
{
  int file;
  int res;

  /* Initialize st as not all fields will be set.  */
  memset (st, 0, sizeof (*st));

  /* Try to open file.  */
  file = _open (name, O_RDONLY);
  if (file == -1)
    /* _open should have already set errno.  */
    return -1;

  /* File opened successfully, infer read permission for owner and assume it is
     a regular file.  */
  st->st_mode |= S_IREAD | S_IFREG;

  /* Fill in more info.  */
  res = __stat_common (file, st);

  _close (file);
  return res;
}
