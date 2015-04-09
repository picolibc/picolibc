	.file	"memchr.S"

	.section .text
	
	.global  _memchr
	.type	 _memchr,@function
_memchr:
	;; R1: string pointer
	;; R2: byte sought
	;; R3: max number to scan
#ifdef __RX_DISALLOW_STRING_INSNS__
	mov.b	r2, r2		; The mov.b below sign extends as it loads, so make sure that r2 is sign-extended as well.
2:	cmp	#0, r3
	beq	1f
	sub	#1, r3
	mov.b	[r1+], r5
	cmp	r5, r2
	bne	2b

	sub	#1, r1		; We have found a match, bit now R1 points to the byte after the match.
1:	rts
#else
	cmp	#0, r3		; If r3 is 0 suntil.b will do nothing and not set any flags...
	stz     #1, r1		; ...so store 1 into r1.  It will be decremented by the SUB later.
	suntil.b    		; Search until *r1 == r2 or r3 bytes have been examined.
	stnz	#1, r1		; If no match was found return NULL.
	sub	#1, r1		; suntil.b leaves r1 pointing at the address *after* the match.
	rts
#endif

	.size _memchr, . - _memchr
	
