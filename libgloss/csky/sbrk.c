/* Copyright (c) 2020  C-SKY Microsystems All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <errno.h>
/*
   + * sbrk -- changes heap size size. Get nbytes more
   + *         RAM. We just increment a pointer in what's
   + *         left of memory on the board.
   + */

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
