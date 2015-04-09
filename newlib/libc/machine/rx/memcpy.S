	.file	"memcpy.S"

	.section .text
	.global  _memcpy
	.type	 _memcpy,@function
_memcpy:
#ifdef __RX_DISALLOW_STRING_INSNS__
	/* Do not use the string instructions - they might prefetch
	   bytes from outside of valid memory.  This is particularly
	   dangerous in I/O space.  */

	;; FIXME: It would be more space efficient to just branch to _memmove...
	
	cmp	 #0, r3	      	; If the count is zero, do nothing
	beq	 1f
	
	mov	 r1, r14	; Save a copy of DEST

2:	mov.b	 [r2+], r5
	mov.b	 r5, [r14+]
	sub	 #1, r3
	bne	 2b
	
1:	rts
#else
	mov	r1, r4		; Save a copy of DEST
	smovf	    		; Copy R2 (source) to R1 (dest).  Stop after R3 bytes.
	mov	r4, r1		; Return DEST
	rts
#endif
	.size _memcpy, . - _memcpy
