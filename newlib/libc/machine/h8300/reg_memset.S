#include "defines.h"

#ifdef __H8300H__
	.h8300h
#endif

#ifdef __H8300S__
	.h8300s
#endif


	; dst A0
	; src A1
	; len A2

	.global ___reg_memset
___reg_memset:
	MOVP	A0P,A3P
	MOVP	A2P,A2P
	beq	noset

memloop:
	mov.b	A1L,@A0P
	adds	#1,A0P
	subs	#1,A2P
	MOVP	A2P,A2P
	bne	memloop

noset:
	MOVP	A3P,A0P
	rts

