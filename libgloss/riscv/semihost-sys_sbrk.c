/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
/* Semihosting requires that sbrk be implemented without a syscall.  */
extern char _end[];               /* _end is set in the linker command file.  */
char *heap_ptr;

/*
 * sbrk -- changes heap size size.  Get nbytes more
 *         RAM.  We just increment a pointer in what's
 *         left of memory on the board.
 */
char *
_sbrk (nbytes)
     int nbytes;
{
  char *base;

  if (!heap_ptr)
    heap_ptr = (char *)&_end;
  base = heap_ptr;
  heap_ptr += nbytes;

  return base;
}
