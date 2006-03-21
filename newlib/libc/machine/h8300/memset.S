#include "setarch.h"

#include "defines.h"

; A0P pointer to cursor
; A1P thing to copy
	.global	_memset

_memset:

;	MOVP	@(2/4,r7),A2P	; dst
;	MOVP	@(4/8,r7),A1	; src thing
;	MOVP	@(6/12,r7),A3P	; len

	MOVP	A2P,A2P
	beq	quit

	; A3 points to the end of the area
	MOVP	A0P,A3P
	ADDP	A2P,A3P

	; see if we can do it in words
	; by oring in the start of the buffer to the end address

	or	A0L,A2L
	btst	#0,A2L
	bne	byteloop
	
	; we can do it a word at a time

	mov.b	A1L,A1H	

wordloop:
	mov.w	A1,@-A3P
	CMPP	A3P,A0P
	bne	wordloop
quit:	rts	

byteloop:
	mov.b	A1L,@-A3P
	CMPP	A3P,A0P
	bne	byteloop
	rts

