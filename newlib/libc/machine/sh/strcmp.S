! Entry: r4: destination
!        r5: source
! Exit:  r0: result
!        r1-r2,r4-r5: clobbered

#include "asm.h"

ENTRY(strcmp)
	mov     r4,r0
	or      r5,r0
	tst	#3,r0
	bf	L_setup_char_loop
	mov	#0,r0
#ifdef DELAYED_BRANCHES
	mov.l	@r4+,r1
	.align  2
Longword_loop:
	mov.l	@r5+,r2
	cmp/str	r0,r1
	bt	Longword_loop_end
	cmp/eq	r1,r2
	bt.s	Longword_loop
	mov.l	@r4+,r1
	add #-4, r4
Longword_loop_end:
	add #-4, r4
	add #-4, r5
L_setup_char_loop:
	mov.b	@r4+,r0
	.align  2
L_char_loop:
	mov.b	@r5+,r1
	tst	r0,r0
	bt	L_return
	cmp/eq	r0,r1
	bt.s L_char_loop
	mov.b	@r4+,r0
	add	#-2,r4
	mov.b	@r4,r0
#else /* ! DELAYED_BRANCHES */
	.align  2
Longword_loop:
	mov.l	@r4+,r1
	mov.l	@r5+,r2
	cmp/str	r0,r1
	bt	Longword_loop_end
	cmp/eq	r1,r2
	bt	Longword_loop
Longword_loop_end:
	add #-4, r4
	add #-4, r5
	.align  2
L_setup_char_loop:
L_char_loop:
	mov.b	@r4+,r0
	mov.b	@r5+,r1
	tst	r0,r0
	bt	L_return
	cmp/eq	r0,r1
	bt L_char_loop
#endif
L_return:
	extu.b	r0,r0
	extu.b	r1,r1
	rts
	sub	r1,r0
