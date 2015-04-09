	.file	"strncat.S"

	.section .text
	.global  _strncat
	.type	 _strncat,@function
_strncat:
	;; On entry: r1 => Destination
	;;           r2 => Source
	;; 	     r3 => Max number of bytes to copy
#ifdef __RX_DISALLOW_STRING_INSNS__
	cmp	#0, r3		; If max is zero we have nothing to do.
	beq	2f
	
	mov 	r1, r4		; Leave the desintation pointer intact for the return value.
	
1:	mov.b	[r4+], r5	; Find the NUL byte at the end of the destination.
	cmp	#0, r5
	bne	1b

	sub	#1, r4

3:	mov.b	[r2+], r5	; Copy bytes from the source into the destination ...
	mov.b	r5, [r4+]
	cmp	#0, r5		; ... until we reach a NUL byte ...
	beq 	2f
	sub	#1, r3
	bne	3b		; ... or we have copied N bytes.
	
2:	rts
#else
	mov 	r1, r4		; Save a copy of the dest pointer.
	mov 	r2, r5		; Save a copy of the source pointer.
	mov 	r3, r14		; Save a copy of the byte count.
	
	mov	#0,  r2		; Search for the NUL byte.
	mov 	#-1, r3		; Search until we run out of memory.
	suntil.b		; Find the end of the destination string.
	sub	#1, r1		; suntil.b leaves r1 pointing to the byte beyond the NUL.

	mov	r14, r3		; Restore the limit on the number of bytes copied.
	mov	r5, r2		; Restore the source pointer.
	mov	r1, r5		; Save a copy of the dest pointer.
	smovu			; Copy source to destination.

	add	#0, r14, r3	; Restore the number of bytes to copy (again), but this time set the Z flag as well.
	beq	1f  		; If we copied 0 bytes then we already know that the dest string is NUL terminated, so we do not have to do anything.
	mov	#0, r2		; Otherwise we must check to see if a NUL byte
	mov	r5, r1		;  was included in the bytes that were copied.
	suntil.b
	beq	1f		; Z flag is set if a match was found.
	add	r14, r5		; Point at byte after end of copied bytes.
	mov.b	#0, [r5]	; Store a NUL there.
1:	
	mov	r4, r1		; Return the original dest pointer.
	rts
#endif
	.size _strncat, . - _strncat
	
