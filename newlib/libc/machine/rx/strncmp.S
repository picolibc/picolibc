	.file	"strncmp.S"

	.section .text
	.global  _strncmp
	.type	 _strncmp,@function
_strncmp:
	;; R1: string1
	;; R2: string2
	;; R3: max number of bytes to compare
#ifdef __RX_DISALLOW_STRING_INSNS__
	cmp	#0, r3		; For a length of zero, return zero
	beq	4f

2:	mov.b	[r1+], r4
	mov.b	[r2+], r5
	cmp	#0, r4
	beq	3f
	cmp	#0, r5
	beq	3f
	sub	#1, r3
	beq	3f
	cmp	r4, r5
	beq	2b

3:	and	#0xff, r4	; We need to perform an unsigned comparison of the bytes.
	and	#0xff, r5
	sub	r5, r4, r1
	rts

4:	mov	#0, r1
	rts
#else
	scmpu			; Perform the string comparison
	bnc	1f		; If Carry is not set skip over
	scne.L	r1		; Set result based on Z flag
	rts			;
1:				;
	mov	#-1,r1		; Carry not set, result should be negative
	rts			;
#endif
	.size _strncmp, . - _strncmp
