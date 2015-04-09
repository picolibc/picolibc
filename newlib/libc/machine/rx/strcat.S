	.file	"strcat.S"

	.section .text
	.global  _strcat
	.type	 _strcat,@function
_strcat:
	;; On entry: r1 => Destination
	;;           r2 => Source
#ifdef __RX_DISALLOW_STRING_INSNS__
	mov 	r1, r4		; Save a copy of the dest pointer.
	
1:	mov.b	[r4+], r5	; Find the NUL byte at the end of R4.
	cmp	#0, r5
	bne	1b

	sub	#1, r4		; Move R4 back to point at the NUL byte.

2:	mov.b	[r2+], r5	; Copy bytes from R2 to R4 until we reach a NUL byte.
	mov.b	r5, [r4+]
	cmp	#0, r5
	bne 	2b

	rts
#else
	mov 	r1, r4		; Save a copy of the dest pointer.
	mov 	r2, r5		; Save a copy of the source pointer.
	
	mov	#0,  r2		; Search for the NUL byte.
	mov 	#-1, r3		; Limit on the number of bytes examined.
	suntil.b		; Find the end of the destination string.
	sub	#1,  r1		; suntil.b leaves r1 pointing to the byte beyond the match.

	mov	#-1, r3		; Set a limit on the number of bytes copied.
	mov	r5,  r2		; Restore the source pointer.
	smovu			; Copy source to destination

	mov	r4, r1		; Return the original dest pointer.
	rts
#endif
	.size _strcat, . - _strcat
	
