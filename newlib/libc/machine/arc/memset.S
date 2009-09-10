/* Copyright (C) 2007 Free Software Foundation, Inc.
   This file is free software; you can redistribute it and/or modify
   it under the same terms as newlib/libc/string/memset.c .  */

#include "asm.h"

#ifndef __A4__

/* To deal with alignment/loop issues, SMALL must be at least 2.  */
#define SMALL	7

	.global	__strncpy_bzero
	.hidden __strncpy_bzero
/* __strncpy_bzero provides the following interface to strncpy:
  r0: return value
  r2: zeroing length
  r3: zeroing start address
  No attempt is made here for __strncpy_memset to speed up aligned
  cases, because the copying of a string presumably leaves start address
  and length alignment for the zeroing randomly distributed.  */

ENTRY(memset)
#ifndef	__ARC700__
#undef SMALL
#define SMALL 8 /* Even faster if aligned.  */
	brls.d	r2,SMALL,.Ltiny
#endif
	mov_s	r3,r0
	or	r12,r0,r2
	bmsk.f	r12,r12,1
	extb_s	r1,r1
	asl	r12,r1,8
	beq.d	.Laligned
	or_s	r1,r1,r12
#ifdef __ARC700__
	brls	r2,SMALL,.Ltiny
#endif
.Lnot_tiny:
	add_s	r12,r2,r0
	stb	r1,[r12,-1]
	bclr_l	r12,r12,0
	stw	r1,[r12,-2]
	bmsk.f	r12,r3,1
	add_s	r2,r2,r12
	sub.ne	r2,r2,4
	stb.ab	r1,[r3,1]
	bclr_s	r3,r3,0
	stw.ab	r1,[r3,2]
	bclr_s	r3,r3,1
.Laligned:	; This code address should be aligned for speed.
#ifdef __ARC700__
	asl	r12,r1,16
	lsr.f	lp_count,r2,2
	or_s	r1,r1,r12
	lpne	.Loop_end
	st.ab	r1,[r3,4]
.Loop_end:
	j_s	[blink]
#else /* !__ARC700 */
	lsr.f	lp_count,r2,3
	asl	r12,r1,16
	or_s	r1,r1,r12
	lpne	.Loop_end
	st.ab	r1,[r3,4]
	st.ab	r1,[r3,4]
.Loop_end:
	jcc	[blink]
	j_s.d	[blink]
	st_s	r1,[r3]
#endif /* !__ARC700 */

#ifdef __ARC700__
	.balign	4
__strncpy_bzero:
	brhi.d	r2,17,.Lnot_tiny
	mov_l	r1,0
.Ltiny:
	mov.f	lp_count,r2
	lpne	.Ltiny_end
	stb.ab	r1,[r3,1]
.Ltiny_end:
	j_s	[blink]
#else /* !__ARC700__ */
#if SMALL > 8
FIXME
#endif
	.balign	4
__strncpy_bzero:
	brhi.d	r2,17,.Lnot_tiny
	mov_s	r1,0
.Ltiny:
	sub_s	r2,r2,11
	sub1	r12,pcl,r2
	j_s	[r12]
	stb_s	r1,[r3,7]
	stb_s	r1,[r3,6]
	stb_s	r1,[r3,5]
	stb_s	r1,[r3,4]
	stb_s	r1,[r3,3]
	stb_s	r1,[r3,2]
	stb_s	r1,[r3,1]
	stb_s	r1,[r3]
	j_s	[blink]
#endif /* !__ARC700 */
ENDFUNC(memset)
#endif /* !__A4__ */
