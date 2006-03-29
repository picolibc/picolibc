#include "defines.h"

#ifdef __H8300H__
	.h8300h
#endif

#ifdef __H8300S__
	.h8300s
#endif

	.global _memcpy
_memcpy:
;	MOVP	@(2/4,r7),A0P	; dst
;	MOVP	@(4/8,r7),A1P	; src
;	MOVP	@(6/12,r7),A2P	; len

	MOVP	A0P,A3P	; keep copy of final dst
	ADDP	A2P,A0P	; point to end of dst
	CMPP	A0P,A3P	; see if anything to do
	beq	quit

	ADDP	A2P,A1P	; point to end of src

	; lets see if we can do this in words
	or	A0L,A2L	; or in the dst address
	or	A3L,A2L	; or the length 
	or	A1L,A2L	; or the src address
	btst	#0,A2L	; see if the lsb is zero
	bne	byteloop

wordloop:
	subs	#2,A1P		; point to word
	mov.w	@A1P,A2		; get word
	mov.w	A2,@-A0P	; save word
	CMPP	A0P,A3P		; at the front again ?
	bne 	wordloop
	rts

byteloop:
	subs	#1,A1P		; point to byte
	mov.b	@A1P,A2L	; get byte
	mov.b	A2L,@-A0P	; save byte
	CMPP	A0P,A3P 	; at the front again ?
	bne 	byteloop

	; return with A0 pointing to dst
quit:	rts

