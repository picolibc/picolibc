/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <sys/types.h>

#ifndef RISCV_SEMIHOST_STAT_H
#define RISCV_SEMIHOST_STAT_H

extern int __stat_common (int, struct stat *);
extern int _open (const char *, int, ...);
extern int _close (int);

#endif
