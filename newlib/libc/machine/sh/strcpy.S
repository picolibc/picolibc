! Entry: r4: destination
!        r5: source
! Exit:  r0: destination
!        r1-r2,r5: clobbered

#include "asm.h"

ENTRY(strcpy)
	mov     r4,r0
	or      r5,r0
	tst	#3,r0
	SL(bf, L_setup_char_loop, mov r4,r0)
	mov.l   @r5+,r1
	mov     #0,r2
	cmp/str r2,r1
	SL(bt, Longword_loop_end, sub r5,r0)
	.align  2
Longword_loop:
	mov.l   r1,@(r0,r5)
	mov.l   @r5+,r1
	cmp/str r2,r1
	bt      Longword_loop_end
	mov.l   r1,@(r0,r5)
	mov.l   @r5+,r1
	cmp/str r2,r1
	bf      Longword_loop
Longword_loop_end:
	add	#-4,r5
	add	#3,r0
	.align  2
L_char_loop:
	mov.b	@r5+,r1
L_char_loop_start:
	tst	r1,r1
	SL(bf, L_char_loop, mov.b r1,@(r0,r5))
	rts
	mov r4,r0
L_setup_char_loop:
	mov.b	@r5+,r1
	bra L_char_loop_start
	sub r5,r0
