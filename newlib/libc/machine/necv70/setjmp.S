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
	.globl	_setjmp
_setjmp:
	mov.w	[ap],r0
	mov.d	r15,[r0+]
	mov.d	r17,[r0+]
	mov.d	r19,[r0+]
	mov.d	r21,[r0+]
	mov.d	r23,[r0+]
	mov.w	-0x4[ap],[r0+]
	mov.w	fp,[r0+]
	mov.w	ap,[r0+]
	mov.w	-0x8[ap],[r0]
	xor.w	r0,r0
	ret	#0x0

	.globl	_longjmp
_longjmp:
	 mov.w	  0x4[ap],r0
	 mov.w	  [ap],r1
	 mov.d	  [r1+],r15
	 mov.d	  [r1+],r17
	 mov.d	  [r1+],r19
	 mov.d	  [r1+],r21
	 mov.d	  [r1+],r23
	 mov.d	  [r1+],ap
	 mov.w	  [r1+],sp
	 test.w	  r0
	 jne	  noz
	 mov.w	  #0x1,r0
noz:	 jmp	  [0x0[r1]]


	
