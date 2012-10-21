/* sbrk for MMIXware.

   Copyright (C) 2001, 2012 Hans-Peter Nilsson

   Permission to use, copy, modify, and distribute this software is
   freely granted, provided that the above copyright notice, this notice
   and the following disclaimer are preserved with no changes.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sys/syscall.h"

extern char *_Sbrk_high;

/* When the program is loaded, the first location in Pool_Segment holds
   the first available octabyte in the memory pool (a.k.a. the heap);
   somewhere after the command-line parameters.  We don't have to
   initialize that location, we can just use it straight up as a pointer;
   just point the symbol there.

   This file will be compiled with -no-builtin-syms, so we need to define
   Pool_Segment and any other symbols that would be predefined in mmixal.  */

__asm__ (" .global _Sbrk_high\n"
	 "_Sbrk_high	IS	Pool_Segment\n"
	 "Pool_Segment	IS	0x40<<56");

caddr_t
_sbrk (size_t incr)
{
  char *prev_heap_end;

  prev_heap_end = _Sbrk_high;

  /* A simulator that requires explicit memory allocation is expected
     to hook that to the PRELD data prefetch insn, which is otherwise
     typically a nop.  */
  if ((long) incr > 0)
    {
      size_t n = incr;
      char *p = prev_heap_end;
#define A(N) __asm__ ("preld " #N ",%0,0" : : "r" (p))
#define PRELDOWNTO(N) while (n >= N + 1) { A(N); n -= N + 1; p += N + 1; }

      PRELDOWNTO (255);
      PRELDOWNTO (31);
      PRELDOWNTO (3);
      PRELDOWNTO (0);
    }

  _Sbrk_high += incr;
  return (caddr_t) prev_heap_end;
}
