	.file	"strncpy.S"

	.section .text
	.global  _strncpy
	.type	 _strncpy,@function
_strncpy:
	mov	r1, r4		; Save a copy of the dest pointer.
	mov	r3, r5		; Save a copy of the byte count
	smovu	    		; Copy the bytes
	cmp	#0, r3		; If we have copied the number of bytes requested
	beq	1f  		;  then skip the next bit:
	add	r4, r5, r1	; Point to the last byte that we are supposed to write.
	sub	r3, r1		; Subtract the number of bytes left to be written.
	mov	#0, r2		; Fill the remaining bytes with NULs,
	sstr.b
1:
	mov	r4, r1		; Return the destination pointer
	rts
