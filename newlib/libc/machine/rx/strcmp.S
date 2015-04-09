	.file	"strcmp.S"

	.section .text
	
	.global  _strcmp
	.type	 _strcmp,@function
_strcmp:
#ifdef __RX_DISALLOW_STRING_INSNS__
2:	mov.b	[r1+], r4
	mov.b	[r2+], r5
	cmp	#0, r4
	beq	3f
	cmp	#0, r5
	beq	3f
	cmp	r4, r5
	beq	2b

3:	and	#0xff, r4	; We need to perform an unsigned comparison of the bytes.
	and	#0xff, r5
	sub	r5, r4, r1
	rts
#else
	mov	#-1, r3		; Strictly speaking this is incorrect, but I doubt if anyone will ever know.
	scmpu			; Perform the string comparison
	bnc	1f		; If Carry is not set skip over
	scne.L	r1		; Set result based on Z flag
	rts			;
1:				;
	mov	#-1,r1		; Carry not set, result should be negative
	rts			;
#endif
	.size _strcmp, . - _strcmp
	
