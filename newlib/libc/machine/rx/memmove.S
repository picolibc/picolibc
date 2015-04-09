	.file	"memmove.S"

	.section .text
	.global  _memmove
	.type	 _memmove,@function
_memmove:
	;; R1: DEST
	;; R2: SRC
	;; R3: COUNT
#ifdef __RX_DISALLOW_STRING_INSNS__
	/* Do not use the string instructions - they might prefetch
	   bytes from outside of valid memory.  This is particularly
	   dangerous in I/O space.  */

	cmp	 #0, r3	      	; If the count is zero, do nothing
	beq	 4f
	
	cmp	 r1, r2
	blt	 3f		; If SRC < DEST copy backwards

	mov	 r1, r14	; Save a copy of DEST

5:	mov.b	 [r2+], r5
	mov.b	 r5, [r14+]
	sub	 #1, r3
	bne	 5b
	
4:	rts

3:	add	 r3, r1
	add	 r3, r2

6:	mov.b	 [-r2], r5
	mov.b	 r5, [-r1]
	sub	 #1, r3
	bne	 6b

	rts
#else	
	mov	r1, r4		; Save a copy of DEST
	cmp	r1, r2
	blt	2f		; If SRC (r2) is less than DEST (r1) then copy backwards
	smovf
1:	
	mov	r4, r1		; Return DEST
	rts
2:
	add	r3, r1		; The SMOVB instructions requires the DEST in r1 and the 
	add	r3, r2		; SRC in r2 but it needs them to point the last bytes of
	sub	#1, r2		; the regions involved not the first bytes, hence these
	sub	#1, r1		; additions and subtractions.
	smovb
	bra	1b

#endif /* SMOVF allowed.  */

	.size _memmove, . - _memmove
