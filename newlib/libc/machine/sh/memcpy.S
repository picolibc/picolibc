!
! Fast SH memcpy
!
! by Toshiyasu Morita (tm@netcom.com)
! hacked by J"orn Rernnecke (amylaar@cygnus.co.uk) ("o for o-umlaut)
!
! Entry: r4: destination pointer
!        r5: source pointer
!        r6: byte count
!
! Exit:  r0: destination pointer
!        r1-r7: trashed
!
! Notes: Usually one wants to do small reads and write a longword, but
!        unfortunately it is difficult in some cases to concatanate bytes
!        into a longword on the SH, so this does a longword read and small
!        writes.
!
! This implementation makes two assumptions about how it is called:
!
! 1.: If the byte count is nonzero, the address of the last byte to be
!     copied is unsigned greater than the address of the first byte to
!     be copied.  This could be easily swapped for a signed comparison,
!     but the algorithm used needs some comparison.
!
! 2.: When there are two or three bytes in the last word of an 11-or-bore
!     bytes memory chunk to b copied, the rest of the word can be read
!     without size effects.
!     This could be easily changed by increasing the minumum size of
!     a fast memcpy and the amount subtracted from r7 before L_2l_loop be 2,
!     however, this would cost a few extra cyles on average.
!

#include "asm.h"

ENTRY(memcpy)
#ifdef __LITTLE_ENDIAN__
	! Little endian version copies with increasing addresses.
	mov r4,r3	! Save return value
	mov #11,r0	! Check if small number of bytes
	cmp/hs r0,r6
			! r6 becomes src end address
	SL(bf, L_small, add r5,r6)
	mov #1,r1
	tst r1,r5	! check if source even
	SL(bt, L_even, mov r6,r7)
	mov.b @r5+,r0	! no, make it even.
	mov.b r0,@r4
	add #1,r4
L_even:	tst r1,r4	! check if destination is even
	add #-3,r7
	SL(bf, L_odddst, mov #2,r1)
	tst r1,r4	! check if destination is 4-byte aligned
	mov r4,r0
	SL(bt, L_al4dst, sub r5,r0)
	mov.w @r5+,r2
	mov.w r2,@r4
	! add #2,r4  r4 is dead here.
L_al4dst:
	tst r1,r5
	bt L_al4both
	mov.w @r5+,r1
	swap.w r1,r1
	add #-6,r0
	add #-6,r7	! r7 := src end address minus 9.
	.align 2
L_2l_loop:
	mov.l @r5+,r2 ! Read & write two longwords per iteration
	xtrct r2,r1
	mov.l r1,@(r0,r5)
	cmp/hs r7,r5
	mov.l @r5+,r1
	xtrct r1,r2
	mov.l r2,@(r0,r5)
	bf L_2l_loop
	add #-2,r5
	bra  L_cleanup
	add #5,r0
L_al4both:
	add #-4,r0
	.align 2
L_al4both_loop:
	mov.l @r5+,r4   ! Read longword, write longword per iteration
	cmp/hs r7,r5
	SL(bf, L_al4both_loop, mov.l r4,@(r0,r5))

	bra L_cleanup
	add #3,r0

L_odddst:
	tst r1,r5
	SL(bt, L_al4src, add #-1,r4)
	mov.w @r5+,r0
	mov.b r0,@(1,r4)
	shlr8 r0
	mov.b r0,@(2,r4)
	add #2,r4
L_al4src:
	.align 2
L_odd_loop:
	mov.l @r5+,r0   ! Read longword, write byte, word, byte per iteration
	cmp/hs r7,r5
	mov.b r0,@(1,r4)
	shlr8 r0
	mov.w r0,@(2,r4)
	shlr16 r0
	mov.b r0,@(4,r4)
	SL(bf, L_odd_loop, add #4,r4)
	.align 2 ! avoid nop in more frequently executed code.
L_cleanup2:
	mov	r4,r0
	sub	r5,r0
L_cleanup:
	cmp/eq	r6,r5
	bt	L_ready
	.align 2
L_cleanup_loop:
	mov.b	@r5+,r1
	cmp/eq	r6,r5
	mov.b	r1,@(r0,r5)
	bf	L_cleanup_loop
L_ready:
	rts
	mov	r3,r0
L_small:
	bra L_cleanup2
	add #-1,r4
#else
	! Big endian version copies with decreasing addresses.
	mov r4,r0
	add r6,r0
	sub r4,r5
	mov #11,r1
	cmp/hs r1,r6
	SL(bf, L_small, add #-1,r5)
	mov r5,r3
	add r0,r3
	shlr r3
	SL(bt, L_even,
	mov r4,r7)
	mov.b @(r0,r5),r2
	add #-1,r3
	mov.b r2,@-r0
L_even:
	tst #1,r0
	add #-1,r5
	SL(bf, L_odddst, add #8,r7)
	tst #2,r0
	bt L_al4dst
	add #-1,r3
	mov.w @(r0,r5),r1
	mov.w r1,@-r0
L_al4dst:
	shlr r3
	bt L_al4both
	mov.w @(r0,r5),r1
	swap.w r1,r1
	add #4,r7
	add #-4,r5
	.align 2
L_2l_loop:
	mov.l @(r0,r5),r2
	xtrct r2,r1
	mov.l r1,@-r0
	cmp/hs r7,r0
	mov.l @(r0,r5),r1
	xtrct r1,r2
	mov.l r2,@-r0
	bt L_2l_loop
	bra L_cleanup
	add #5,r5

	nop ! avoid nop in executed code.
L_al4both:
	add #-2,r5
	.align 2
L_al4both_loop:
	mov.l @(r0,r5),r1
	cmp/hs r7,r0
	SL(bt, L_al4both_loop,
	mov.l r1,@-r0)
	bra L_cleanup
	add #3,r5

	nop ! avoid nop in executed code.
L_odddst:
	shlr r3
	bt L_al4src
	mov.w @(r0,r5),r1
	mov.b r1,@-r0
	shlr8 r1
	mov.b r1,@-r0
L_al4src:
	add #-2,r5
	.align 2
L_odd_loop:
	mov.l @(r0,r5),r2
	cmp/hs r7,r0
	mov.b r2,@-r0
	shlr8 r2
	mov.w r2,@-r0
	shlr16 r2
	mov.b r2,@-r0
	bt L_odd_loop

	add #3,r5
L_cleanup:
L_small:
	cmp/eq r4,r0
	bt L_ready
	add #1,r4
	.align 2
L_cleanup_loop:
	mov.b @(r0,r5),r2
	cmp/eq r4,r0
	mov.b r2,@-r0
	bf L_cleanup_loop
L_ready:
	rts
	nop
#endif
