!
! Fast SH memset
!
! by Toshiyasu Morita (tm@netcom.com)
!
! SH5 code by J"orn Rennecke (joern.rennecke@superh.com)
! Copyright 2002 SuperH Ltd.
!

#include "asm.h"

ENTRY(memset)
#if __SHMEDIA__
	pta/l multiquad, tr0
	andi r2, 7, r22
	ptabs r18, tr2
	mshflo.b r3,r3,r3
	add r4, r22, r23
	mperm.w r3, r63, r3	// Fill pattern now in every byte of r3

	movi 8, r9
	bgtu/u r23, r9, tr0 // multiquad

	ldlo.q r2, 0, r7
	shlli r4, 2, r4
	movi -1, r8
	SHHI r8, r4, r8
	SHHI r8, r4, r8
	mcmv r7, r8, r3
	stlo.q r2, 0, r3
	blink tr2, r63

multiquad:
	pta/l lastquad, tr0
	stlo.q r2, 0, r3
	shlri r23, 3, r24
	add r2, r4, r5
	beqi/u r24, 1, tr0 // lastquad
	pta/l loop, tr1
	sub r2, r22, r25
	andi r5, -8, r20   // calculate end address and
	addi r20, -7*8, r8 // loop end address; This might overflow, so we need
	                   // to use a different test before we start the loop
	bge/u r24, r9, tr1 // loop
	st.q r25, 8, r3
	st.q r20, -8, r3
	shlri r24, 1, r24
	beqi/u r24, 1, tr0 // lastquad
	st.q r25, 16, r3
	st.q r20, -16, r3
	beqi/u r24, 2, tr0 // lastquad
	st.q r25, 24, r3
	st.q r20, -24, r3
lastquad:
	sthi.q r5, -1, r3
	blink tr2,r63

loop:
	alloco r25, 32
	st.q r25, 8, r3
	st.q r25, 16, r3
	st.q r25, 24, r3
	st.q r25, 32, r3
	addi r25, 32, r25
	bgeu/l r8, r25, tr1 // loop

	st.q r20, -40, r3
	st.q r20, -32, r3
	st.q r20, -24, r3
	st.q r20, -16, r3
	st.q r20, -8, r3
	sthi.q r5, -1, r3
	blink tr2,r63
#else /* ! SHMEDIA, i.e. SH1 .. SH4 / SHcompact */
! Entry: r4: destination pointer
!        r5: fill value
!        r6: byte count
!
! Exit:  r0-r3: trashed
!

! This assumes that the first four bytes of the address space (0..3) are
! reserved - usually by the linker script.  Otherwise, we would had to check
! for the case of objects of the size 12..15 at address 0..3 .

#ifdef __SH5__
#define DST r2
#define VAL r3
#define CNT r4
#define TMP r5
#else
#define DST r4
#define VAL r5
#define CNT r6
#define TMP r2
#endif

	mov	#12,r0	! Check for small number of bytes
	cmp/gt	CNT,r0
	mov	DST,r0
	SL(bt, L_store_byte_loop_check0, add DST,CNT)

	tst	#3,r0	! Align destination
	SL(bt,	L_dup_bytes, extu.b r5,r5)
	.balignw 4,0x0009
L_align_loop:
	mov.b	VAL,@r0
	add	#1,r0
	tst	#3,r0
	bf	L_align_loop

L_dup_bytes:	
	swap.b	VAL,TMP	! Duplicate bytes across longword
	or	TMP,VAL
	swap.w	VAL,TMP
	or	TMP,VAL

	add	#-16,CNT

	.balignw 4,0x0009
L_store_long_loop:
	mov.l	VAL,@r0	! Store double longs to memory
	cmp/hs	CNT,r0
	mov.l	VAL,@(4,r0)
	SL(bf, L_store_long_loop, add #8,r0)

	add	#16,CNT

L_store_byte_loop_check0:
	cmp/eq	CNT,r0
	bt	L_exit
	.balignw 4,0x0009
L_store_byte_loop:
	mov.b	VAL,@r0	! Store bytes to memory
	add	#1,r0
	cmp/eq	CNT,r0
	bf	L_store_byte_loop

L_exit:
	rts
	mov	r4,r0
#endif /* ! SHMEDIA */
