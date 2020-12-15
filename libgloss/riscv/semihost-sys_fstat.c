/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <string.h>
#include <sys/stat.h>
#include "semihost_stat.h"

/* Status of an open file.  The sys/stat.h header file required is
   distributed in the include subdirectory for this C library.  */

int
_fstat (int file, struct stat *st)
{
  /* Initialize st as not all fields will be set.  */
  memset (st, 0, sizeof (*st));

  return __stat_common (file, st);
}
