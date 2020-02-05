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
#include "m68kasm.h"

.global SYM (setjmp)
.global SYM (longjmp)

SYM (setjmp):
	moveal sp@(4),a0
	movel sp@(0),a0@(12)
	movel sp,a0@(8)
	moveml d2-d7/a2-a6,a0@(20)
	clrl d0
	rts

SYM (longjmp):
	moveal sp@(4),a0
	movel sp@(8),d0
	bne 1f
	movel &1,d0
1:
	moveml a0@(20),d2-d7/a2-a6
	moveal a0@(8),sp
	movel a0@(12),sp@
	rts

#ifdef M68881
.global SYM (setjmp_68881)
.global SYM (longjmp_68881)

SYM (setjmp_68881):
	moveal sp@(4),a0
	movel sp@(0),a0@(12)
	movel sp,a0@(8)
	moveml d2-d7/a2-a6,a0@(20)
	fmovemx fp2-fp7,a0@(64)
	clrl d0
	rts

SYM (longjmp_68881):
	moveal sp@(4),a0
	fmovemx a0@(64),fp2-fp7
	movel sp@(8),d0
	bne 1f
	movel &1,d0
1:
	moveml a0@(20),d2-d7/a2-a6
	moveal a0@(8),sp
	movel a0@(12),sp@
	rts
#endif
