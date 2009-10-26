	.file	"strcat.S"

	.section .text
	.global  _strcat
	.type	 _strcat,@function
_strcat:
	;; On entry: r1 => Destination
	;;           r2 => Source
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
