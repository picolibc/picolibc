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

	.global ___reg_memset
___reg_memset:
	MOVP	A0P,A3P
	MOVP	A2P,A2P
	beq	noset

memloop:
	mov.b	A1L,@A0P
#ifdef __NORMAL_MODE__
	add	#1,A0P
	sub	#1,A2P
#else
	adds	#1,A0P
	subs	#1,A2P
#endif
	MOVP	A2P,A2P
	bne	memloop

noset:
	MOVP	A3P,A0P
	rts

