#include "defines.h"

#ifdef __H8300H__
	.h8300h
#endif

#ifdef __H8300S__
	.h8300s
#endif

	.section .text
	.align 2
	.global _strcmp
_strcmp:
	MOVP	A0P,A2P
	MOVP	A1P,A3P
.L5:
	mov.b	@A2P+,A1L
	beq	.L3
	mov.b	@A3P+,A0L
	cmp.b	A0L,A1L
	beq	.L5
	subs	#1,A3P
.L3:
	mov.b	@(-1,A2P),A0L
	mov.b	@A3P,A1L
	sub.b	A1L,A0L
	; We have to sign extend the result to 32bits just in case
	; we are using 32bit integers.
#ifdef __H8300H__
	exts.w	r0
	exts.l	er0
#else
#ifdef __H8300S__
	exts.w	r0
	exts.l	er0
#else
	bld	#7,r0l
	subx	r0h,r0h
	subx	r1l,r1l
	subx	r1h,r1h
#endif
#endif
	rts
	.end
