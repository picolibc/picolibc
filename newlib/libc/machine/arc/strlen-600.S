/* Copyright (C) 2007, 2008 Free Software Foundation, Inc.
   This file is free software; you can redistribute it and/or modify
   it under the same terms as newlib/libc/string/strlen.c .  */

#include "asm.h"

#if !defined (__ARC700__) && !defined (__A4__)
/* This code is optimized for the ARC600 pipeline.  */

ENTRY(strlen)
	or	r3,r0,7
	ld	r2,[r3,-7]
	ld.a	r6,[r3,-3]
	mov	r4,0x01010101
	; uses long immediate
#ifdef __LITTLE_ENDIAN__
	asl_s	r1,r0,3
	btst_s	r0,2
	asl	r7,r4,r1
	ror	r5,r4
	sub	r1,r2,r7
	bic_l	r1,r1,r2
	mov.eq	r7,r4
	sub	r12,r6,r7
	bic	r12,r12,r6
	or.eq	r12,r12,r1
	and	r12,r12,r5
	brne	r12,0,.Learly_end
#else /* BIG ENDIAN */
	ror	r5,r4
	btst_s	r0,2
	mov_s	r1,31
	sub3	r7,r1,r0
	sub	r1,r2,r4
	bic_l	r1,r1,r2
	bmsk	r1,r1,r7
	sub	r12,r6,r4
	bic	r12,r12,r6
	bmsk.ne	r12,r12,r7
	or.eq	r12,r12,r1
	and	r12,r12,r5
	brne	r12,0,.Learly_end
#endif /* ENDIAN */

.Loop:
	ld_s	r2,[r3,4]
	ld.a	r6,[r3,8]
	; stall for load result
	sub	r1,r2,r4
	bic_s	r1,r1,r2
	sub	r12,r6,r4
	bic	r12,r12,r6
	or_s	r12,r12,r1
	and	r12,r12,r5
	breq_s	r12,0,.Loop
.Lend:
	and.f	r1,r1,r5
	sub.ne	r3,r3,4
#ifdef __LITTLE_ENDIAN__
	mov.eq	r1,r12
	asr.f	0,r1,8
	bmsk.f	0,r1,15
	sub	r0,r3,r0
	add.cc	r0,r0,1
	jne.d	[blink]
	asl.f	0,r1,9
	j_s.d	[blink]
	sbc	r0,r0,-2
#else /* BIG ENDIAN */
	mov.eq	r2,r6
	asl_s	r2,r2,7
	mov.eq	r1,r12
	bic_s	r1,r1,r2
	asr.f	0,r1,16
	sub	r0,r3,r0
	add.pl	r0,r0,1
	jne.d	[blink]
	add.eq	r0,r0,1
	j_s.d	[blink]
	add.cc	r0,r0,1
#endif /* ENDIAN */

	.balign	4
.Learly_end:
	b.d	.Lend
	sub_s.ne r1,r1,r1
ENDFUNC(strlen)
#endif /* !ARC700 */
