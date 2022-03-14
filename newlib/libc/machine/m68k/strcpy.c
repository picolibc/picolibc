/*
Copyright (c) 1990 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
by the University of California, Berkeley.  The name of the
University may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 *  C library strcpy routine
 *
 *  This routine has been optimized for the CPU32+.
 *  It should run on all 68k machines.
 *
 *  W. Eric Norum
 *  Saskatchewan Accelerator Laboratory
 *  University of Saskatchewan
 *  Saskatoon, Saskatchewan, CANADA
 *  eric@skatter.usask.ca
 */

#include <string.h>

/*
 * Copy bytes using CPU32+ loop mode if possible
 */

#undef strcpy

char *
strcpy (char *to, const char *from)
{
	char *pto = to;
	unsigned int n = 0xFFFF;

	__asm__ volatile ("1:\n"
	     "\tmove.b (%0)+,(%1)+\n"
#if defined(__mcpu32__)
	     "\tdbeq %2,1b\n"
#endif
	     "\tbne.b 1b\n" :
		"=a" (from), "=a" (pto), "=d" (n) :
		 "0" (from),  "1" (pto), "2" (n) :
		 "cc", "memory");
	return to;
}
