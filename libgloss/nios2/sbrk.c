/*
 * sbrk.c
 *
 * Copyright (c) 2018 Mentor Graphics
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <errno.h>
/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */

/* Provided by the linker script.  */
extern char __heap_start[] __attribute__ ((aligned (8)));
extern char __heap_end[] __attribute__ ((aligned (8)));

void *
sbrk (int nbytes)
{
  static char *heap = __heap_start;
  char *base = heap;
  char *new_heap = heap + nbytes;
  
  if (nbytes < 0 || new_heap > __heap_end)
    {
      errno = ENOMEM;
      return (void *)-1;
    }
  heap = new_heap;
  return base;
}
