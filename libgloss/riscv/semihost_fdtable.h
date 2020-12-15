/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <sys/types.h>

#ifndef RISCV_SEMIHOST_FDTABLE_H
#define RISCV_SEMIHOST_FDTABLE_H

extern void __attribute__ ((constructor)) init_semihosting ();
extern int __add_fdentry (int);
extern struct fdentry * __get_fdentry (int);
extern void __remove_fdentry (int);

struct fdentry
{
  int handle;
  off_t pos;
};

#endif
