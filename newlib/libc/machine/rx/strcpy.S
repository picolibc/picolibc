	.file	"strcpy.S"

	.section .text
	.global  _strcpy
	.type	 _strcpy,@function
_strcpy:
	;; R1: dest
	;; R2: source
#ifdef __RX_DISALLOW_STRING_INSNS__
	mov	r1, r4		; Leave the destination address unchanged in the result register.

1:	mov.b	[r2+], r5
	mov.b	r5, [r4+]
	cmp	#0, r5
	bne	1b

	rts
#else
	mov	r1, r4
	mov	#-1, r3		; Strictly speaking this is incorrect, but I doubt if anyone will ever know.
	smovu
	mov	r4, r1
	rts
#endif
	.size _strcpy, . - _strcpy
