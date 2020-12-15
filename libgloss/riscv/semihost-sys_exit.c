/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <machine/syscall.h>
#include "semihost_syscall.h"

#define ADP_Stopped_ApplicationExit 0x20026

/* Exit a program without cleaning up files.  */
void
_exit (int exit_status)
{
#if __riscv_xlen == 32
  syscall_errno (SEMIHOST_exit, (long *) ADP_Stopped_ApplicationExit);
#else
  /* The semihosting exit operation only allows 64-bit targets to report the
     exit code.  */
  long data_block[] = {ADP_Stopped_ApplicationExit, exit_status};
  syscall_errno (SEMIHOST_exit, data_block);
#endif
  while (1);
}
