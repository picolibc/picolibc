/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#ifndef _RISCV_KERNEL_STAT_H
#define _RISCV_KERNEL_STAT_H

#include <sys/types.h>
#include <sys/stat.h>

struct  kernel_stat
{
  unsigned long long st_dev;
  unsigned long long st_ino;
  unsigned int st_mode;
  unsigned int st_nlink;
  unsigned int st_uid;
  unsigned int st_gid;
  unsigned long long st_rdev;
  unsigned long long __pad1;
  long long st_size;
  int st_blksize;
  int __pad2;
  long long st_blocks;
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
  int __glibc_reserved[2];
};

void _conv_stat (struct stat *, struct kernel_stat *);
#endif /* _RISCV_KERNEL_STAT_H */
