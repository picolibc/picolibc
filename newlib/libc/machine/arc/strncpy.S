/* Copyright (C) 2007, 2008 Free Software Foundation, Inc.
   This file is free software; you can redistribute it and/or modify
   it under the same terms as newlib/libc/string/strncpy.c .  */

#include "asm.h"

/* If dst and src are 4 byte aligned, copy 8 bytes at a time.
   If the src is 4, but not 8 byte aligned, we first read 4 bytes to get
   it 8 byte aligned.  Thus, we can do a little read-ahead, without
   dereferencing a cache line that we should not touch.
   Note that short and long instructions have been scheduled to avoid
   branch stalls.
   The beq_s to r3z could be made unaligned & long to avoid a stall
   there, but the it is not likely to be taken often, and it
   would also be likey to cost an unaligned mispredict at the next call.  */
   
#ifdef __ARC700__
#define BRand(a,b,l)	tst	a,b ` bne_l l
#else
#define BRand(a,b,l)	and	a,a,b ` brne_s a,0,l
#endif

ENTRY(strncpy)
	cmp_s	r2,8
	or	r12,r0,r1
	bmsk.cc.f r12,r12,1
	brne.d	r12,0,.Lbytewise
	mov_s	r10,r0
	ld_s	r3,[r1,0]
	mov	r8,0x01010101
	sub	lp_count,r2,1
	bbit0.d	r1,2,.Loop_start
	ror	r11,r8
	sub	r12,r3,r8
	bic_l	r12,r12,r3
	BRand	(r12,r11,.Lr3z)
	mov_s	r4,r3
	ld.a	r3,[r1,4]
	sub	lp_count,lp_count,4
	st.ab	r4,[r10,4]
	.balign 4
.Loop_start:
	lsr.f	lp_count,lp_count,3
	lpne	.Loop_end
	ld.a	r4,[r1,4]
	sub	r12,r3,r8
	bic_s	r12,r12,r3
	BRand	(r12,r11,.Lr3z)
	st.ab	r3,[r10,4]
	sub	r12,r4,r8
	bic	r12,r12,r4
	BRand	(r12,r11,.Lr4z)
	ld.a	r3,[r1,4]
	st.ab	r4,[r10,4]
.Loop_end:
	bcc_s	.Lastword
	ld.a	r4,[r1,4]
	sub	r12,r3,r8
	bic_s	r12,r12,r3
	BRand	(r12,r11,.Lr3z)
	st.ab	r3,[r10,4]
	mov_s	r3,r4
.Lastword:
	and.f	lp_count,r2,3
	mov.eq	lp_count,4
	lp	.Last_byte_end
#ifdef __LITTLE_ENDIAN__
	bmsk.f	r1,r3,7
	lsr.ne	r3,r3,8
#else
	lsr.f	r1,r3,24
	asl.ne	r3,r3,8
#endif
	stb.ab	r1,[r10,1]
.Last_byte_end:
	j_s	[blink]

	.balign	4
.Lr4z:
	mov_l	r3,r4
.Lr3z:
#ifdef __ARC700__
#ifdef __LITTLE_ENDIAN__
	bmsk.f	r1,r3,7
	lsr_s	r3,r3,8
#else
	lsr.f	r1,r3,24
	asl_s	r3,r3,8
#endif
	bne.d	.Lr3z
	stb.ab	r1,[r10,1]
#else /* ! __ARC700__ */
#ifdef __LITTLE_ENDIAN__
	bmsk.f	r1,r3,7
.Lr3z_loop:
	lsr_s	r3,r3,8
	stb.ab	r1,[r10,1]
	bne.d	.Lr3z_loop
	bmsk.f	r1,r3,7
#else
	lsr.f	r1,r3,24
.Lr3z_loop:
	asl_s	r3,r3,8
	stb.ab	r1,[r10,1]
	bne.d	.Lr3z_loop
	lsr.f	r1,r3,24
#endif /* ENDIAN */
#endif /* ! __ARC700__ */
.Lzero_rest:
	; __strncpy_bzero requires:
        ; return value in r0
        ; zeroing length in r2
        ; zeroing start address in r3
	mov_s	r3,r10
	add_s	r2,r2,r0
        b.d	__strncpy_bzero
	sub_s	r2,r2,r3
	
	.balign	4
.Lbytewise:
	sub.f	r2,r2,1
	mov_l	r3,r0
	jcs	[blink]
.Lcharloop:
	ldb.ab	r12,[r1,1]
	beq.d	.Last_byte
	sub.f	r2,r2,1
	brne.d	r12,0,.Lcharloop
	stb.ab	r12,[r3,1]
        b.d	__strncpy_bzero
	stb.ab	r12,[r3,1]
.Last_byte:
	j_s.d	[blink]
	stb_l	r12,[r3]
ENDFUNC(strncpy)
