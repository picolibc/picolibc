	.file	"memmove.S"

	.section .text
	.global  _memmove
	.type	 _memmove,@function
_memmove:
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
