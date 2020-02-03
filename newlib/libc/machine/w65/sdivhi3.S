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
	.global	___sdivhi3

___sdivhi3:
	lda	<r4
	ldx	<r5		

	ldy	#0		!flag positive result
	rol	a
	ror	a
	bpl	L10

	iny			!flag negative result
	eor	#0xFFFF
	inc	a

L10:	pha
	txa
	bpl	L20

	dey			!flag negative/positive result
	eor	#0xFFFF
	inc	a
	tax

L20:	pla
	phy
	jsr	>udv
	ply
	beq	Lend		!if positive result

	eor	#0xFFFF		!negate result
	inc	a

Lend:	sta	<r0
	rtl
