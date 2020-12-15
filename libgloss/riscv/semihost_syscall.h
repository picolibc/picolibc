/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#ifndef _INTERNAL_SYSCALL_H
#define _INTERNAL_SYSCALL_H

extern int errno;

static inline long
__semihost_syscall (long id, long *data_block)
{
  register long a0 asm ("a0") = id;
  register long a1 asm ("a1") = (long) data_block;

  /* RISC-V semihosting trap sequence.  Must be uncompressed and must not
     cross page boundary.  */
  asm volatile (
    ".balign 16             \n"
    ".option push           \n"
    ".option norvc          \n"
    "slli zero, zero, 0x1f  \n"
    "ebreak                 \n"
    "srai zero, zero, 0x7   \n"
    ".option pop            \n"
      : "+r"(a0) : "r"(a1) : "memory");

  return a0;
}

static inline long
__syscall_error ()
{
  errno = __semihost_syscall (SEMIHOST_errno, 0);
  return -1;
}

static inline long
syscall_errno (long id, long *data_block)
{
  long res = __semihost_syscall (id, data_block);
  if (res < 0)
    return __syscall_error ();
  return res;
}

#endif
