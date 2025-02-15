/*
Copyright (c) 2009 Nick Clifton <nickc@redhat.com>
 */
#include <picolibc.h>

#ifdef __RX_ALLOW_STRING_INSNS__
	.file	"strcpy.S"

	.section .text.strcpy
	.global  _strcpy
	.type	 _strcpy,@function
_strcpy:
	;; R1: dest
	;; R2: source
	mov	r1, r4
	mov	#-1, r3		; Strictly speaking this is incorrect, but I doubt if anyone will ever know.
	smovu
	mov	r4, r1
	rts
	.size _strcpy, . - _strcpy
#endif
