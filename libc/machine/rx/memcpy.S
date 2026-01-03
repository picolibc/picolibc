/*
Copyright (c) 2009 Nick Clifton <nickc@redhat.com>
 */
#include <picolibc.h>

#ifdef __RX_ALLOW_STRING_INSNS__
	.file	"memcpy.S"

	.section .text.memcpy
	.global  _memcpy
	.type	 _memcpy,@function
_memcpy:
	mov	r1, r4		; Save a copy of DEST
	smovf	    		; Copy R2 (source) to R1 (dest).  Stop after R3 bytes.
	mov	r4, r1		; Return DEST
	rts
	.size _memcpy, . - _memcpy
#endif
