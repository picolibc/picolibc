/* _exit for MMIXware.

   Copyright (C) 2001 Hans-Peter Nilsson

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

void _exit (int n)
{
  /* Unfortunately, the return status is not returned by Knuth's mmix
     simulator, so it seems in effect ineffective.  We set it anyway;
     there may be a purpose.  */
  __asm__ ("SET $255,%0\n\tTRAP 0,0,0"
	   : /* No outputs.  */
	   : "r" (n)
	   : "memory");
}
