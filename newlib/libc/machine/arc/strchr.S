/* Copyright (C) 2007 Free Software Foundation, Inc.
   This file is free software; you can redistribute it and/or modify
   it under the same terms as newlib/libc/string/strchr.c .  */

#include "asm.h"

/* In order to search for a zero in a W, we calculate
   X := (W - 0x01010101) & ~W & 0x80808080;
   In the little endian case:
     If no byte in W is zero, X will be zero; otherwise, the least significant
     byte of X which is nonzero indicates the least significant byte of W that
     is zero.
   In the big endian case:
     X will be zero iff no byte in W is zero.
     If X is nonzero, to find out which is the most significant zero byte
     in W, we calculate:
     Y := ~(((W | 0x80808080) - 0x01010101) | W) & 0x80808080;
     Each byte in Y is 0x80 if the the corresponding byte in
     W is zero, otherwise that byte of Y is 0.  */

#if !defined(__ARC700__) && !defined(__A4__)
ENTRY(strchr)
	bmsk.f	r2,r0,1
	mov_s	r3,0x01010101
	extb_s	r1,r1
	asl	r5,r1,8
	or	r5,r5,r1
	beq.d	.Laligned
	asl	r4,r5,16
        sub_s   r0,r0,r2
        asl_s   r2,r2,3
#ifdef	__LITTLE_ENDIAN__
        asl     r7,r3,r2
#else
        lsr     r7,r3,r2
#endif
	ld_s	r2,[r0]
	or	r5,r5,r4
	ror	r4,r3
	sub	r12,r2,r7
	bic_s	r12,r12,r2
	and	r12,r12,r4

	brne.d	r12,0,.Lfound0_ua
	xor	r6,r2,r5
	ld.a	r2,[r0,4]
	sub	r12,r6,r7
	bic	r12,r12,r6
	and.f	r7,r12,r4
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	beq.d	.Loop
	and	r12,r12,r4
	b.d	.Lfound_char_ua
#ifdef __LITTLE_ENDIAN__
	btst	r7,7
#else
	asl	r6,r6,7
#endif

	.balign	4
.Laligned:
	ld_s	r2,[r0]
	or	r5,r5,r4
	ror	r4,r3
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	and	r12,r12,r4
.Loop:

	brne.d	r12,0,.Lfound0
	xor	r6,r2,r5
	ld.a	r2,[r0,4]
	sub	r12,r6,r3
	bic	r12,r12,r6
	and.f	r7,r12,r4
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	beq.d	.Loop
	and	r12,r12,r4
; Found searched-for character.  r0 has already advanced to next word.
#ifdef __LITTLE_ENDIAN__
/* We only need the information about the first matching byte
   (i.e. the least significant matching byte) to be exact,
   hence there is no problem with carry effects.  */
.Lfound_char:
	btst	r7,7
.Lfound_char_ua:
	sub_s	r0,r0,4
	add.eq	r0,r0,1
	btst.eq	r7,15
	add.eq	r0,r0,1
	btst.eq	r7,23
	j_s.d	[blink]
	add.eq	r0,r0,1

	.balign	4
.Lfound0_ua:
	mov_l	r3,r7
.Lfound0:
	sub	r2,r6,r3
	bic	r2,r2,r6
	and	r2,r2,r4
	or	r3,r12,r2
	sub_s	r12,r3,1
	xor_s	r3,r3,r12
	tst_s	r2,r3
	lsr	r2,r3,31
	lsr	r12,r3,16
	jeq.d	[blink]
	mov.eq	r0,0
	lsr	r3,r3,8
	sub_s	r2,r2,r12
	sub_s	r2,r2,r3
	bmsk_s	r2,r2,1
	j_s.d	[blink]
	add_s	r0,r0,r2
#else /* BIG ENDIAN */
.Lfound_char:
	asl	r6,r6,7
.Lfound_char_ua:
	sub_s	r0,r0,4
	bic.f	r2,r7,r6
	add.pl	r0,r0,1
	jmi.d	[blink]
	btst_s	r2,23
	add.eq	r0,r0,1
	btst.eq	r2,15
	j_s.d	[blink]
	add.eq	r0,r0,1

; N.B. if we searched for a char zero and found it in the MSB,
; and ignored matches are identical, we will take the early exit
; like for an ordinary found zero - except for the extra stalls at jhi -
; but still compute the right result.
.Lfound0_ua:
	mov_s	r3,r7
.Lfound0:
	asl_s	r2,r2,7
	or	r7,r6,r4
	bic_s	r12,r12,r2
	sub	r2,r7,r3
	or	r2,r2,r6
	bic	r2,r4,r2
	cmp	r12,r2
	mov.hi	r0,0
	btst.ls	r2,31
	jhi.d	[blink]
	add.eq	r0,r0,1
	btst.eq	r2,23
	add.eq	r0,r0,1
	btst.eq	r2,15
	j_s.d	[blink]
	add.eq	r0,r0,1
#endif /* ENDIAN */
ENDFUNC(strchr)
#endif /* !__ARC_700__ */
