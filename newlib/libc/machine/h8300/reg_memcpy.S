#include "defines.h"

#ifdef __H8300H__
#ifdef __NORMAL_MODE__
	.h8300hn
#else
	.h8300h
#endif
#endif

#ifdef __H8300S__
#ifdef __NORMAL_MODE__
	.h8300sn
#else
	.h8300s
#endif
#endif


	; dst A0
	; src A1
	; len A2
	.global ___reg_memcpy
___reg_memcpy:

	MOVP	A0P,A3P	; keep copy of result
	ADDP	A2P,A0P	; point to end of dst
	ADDP	A2P,A1P	; point to end of src
	CMPP	A0P,A3P	; see if anything to do
	beq	quit


loop:	subs	#1,A1P		; point to byte
	mov.b	@A1P,A2L	; get byte
	mov.b	A2L,@-A0P	; save byte
	CMPP	A0P,A3P 	; at the front again ?
	bne 	loop

	; return with A0 pointing to dst
quit:	rts

