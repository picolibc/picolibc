/*
Copyright (c) 2009 Nick Clifton <nickc@redhat.com>
 */
#include <picolibc.h>

#ifdef __RX_ALLOW_STRING_INSNS__
	.file	"strncmp.S"

	.section .text.strncmp
	.global  _strncmp
	.type	 _strncmp,@function
_strncmp:
	;; R1: string1
	;; R2: string2
	;; R3: max number of bytes to compare
	setpsw	z		; Set flags in case len is 0
	setpsw	c
	scmpu			; Perform the string comparison
	bnc	1f		; If Carry is not set skip over
	scne.L	r1		; Set result based on Z flag
	rts			;
1:				;
	mov	#-1,r1		; Carry not set, result should be negative
	rts			;
	.size _strncmp, . - _strncmp
#endif
