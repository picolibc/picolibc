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
 *  C library strlen routine
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
 * Test bytes using CPU32+ loop mode if possible.
 */
size_t
strlen (const char *str)
{
	unsigned int n = ~0;
	const char *cp = str;

	__asm__ volatile ("1:\n"
	     "\ttst.b (%0)+\n"
#if defined(__mcpu32__)
	     "\tdbeq %1,1b\n"
#endif
	     "\tbne.b 1b\n" :
		"=a" (cp), "=d" (n) :
		 "0" (cp),  "1" (n) :
		 "cc");
	return (cp - str) - 1;
}
