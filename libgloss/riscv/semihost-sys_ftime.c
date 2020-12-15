/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include <sys/timeb.h>
#include "semihost_syscall.h"

/* Get the current time.  */
int
_ftime (struct timeb *tp)
{
  tp->time = syscall_errno (SEMIHOST_time, 0);
  tp->millitm = 0;
  return 0;
}
