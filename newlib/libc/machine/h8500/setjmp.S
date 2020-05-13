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
!
	.file	"setjmp.S"
	.section .text

	.global	_setjmp
_setjmp:
	mov.w	r7,@r0+
	mov.w	r6,@r0+
	mov.w	r5,@r0+
	mov.w	@sp,r1
	mov.w	r1,@r0
	clr.w	r0
	rts

	.global	_longjmp
_longjmp:
	mov.w	@r0+,r7
	mov.w	@r0+,r6
	mov.w	@r0+,r5
	mov.w	@r0,r2
	mov.w	r2,@sp
	mov.w	r1,r0
	tst.w	r0
	bne	.L1
	mov.w	#1,r0
.L1:
	rts
