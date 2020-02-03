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
#include "setarch.h"

#include "defines.h"

	; dst A0
	; src A1
	; len A2
	.global ___reg_memcpy
___reg_memcpy:

	MOVP	A0P,A3P	; keep copy of result
	ADDP	A2P,A0P	; point to end of dst
	ADDP	A2P,A1P	; point to end of src
	CMPP	A0P,A3P	; see if anything to do
	beq	quit

loop:
#ifdef __NORMAL_MODE__
	sub     #1,A1P		; point to byte
#else
	subs    #1,A1P          ; point to byte
#endif
	mov.b	@A1P,A2L	; get byte
	mov.b	A2L,@-A0P	; save byte
	CMPP	A0P,A3P 	; at the front again ?
	bne 	loop

	; return with A0 pointing to dst
quit:	rts

