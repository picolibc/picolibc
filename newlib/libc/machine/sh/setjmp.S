#include "asm.h"

ENTRY(setjmp)
#if defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(__SH4_SINGLE_ONLY__)
	add	#(13*4),r4
#else
	add	#(9*4),r4
#endif

	sts.l	pr,@-r4

#if defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(__SH4_SINGLE_ONLY__)
	fmov.s	fr15,@-r4	! call saved floating point registers
	fmov.s	fr14,@-r4
	fmov.s	fr13,@-r4
	fmov.s	fr12,@-r4
#endif

	mov.l	r15,@-r4	! call saved integer registers
	mov.l	r14,@-r4
	mov.l	r13,@-r4
	mov.l	r12,@-r4

	mov.l	r11,@-r4
	mov.l	r10,@-r4
	mov.l	r9,@-r4
	mov.l	r8,@-r4

	rts
	mov    #0,r0

ENTRY(longjmp)
	mov.l	@r4+,r8
	mov.l	@r4+,r9
	mov.l	@r4+,r10
	mov.l	@r4+,r11

	mov.l	@r4+,r12
	mov.l	@r4+,r13
	mov.l	@r4+,r14
	mov.l	@r4+,r15

#if defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(__SH4_SINGLE_ONLY__)
	fmov.s	@r4+,fr12	! call saved floating point registers
	fmov.s	@r4+,fr13
	fmov.s	@r4+,fr14
	fmov.s	@r4+,fr15
#endif

	lds.l	@r4+,pr

	mov	r5,r0
	tst	r0,r0
	bf	retr4
	movt	r0
retr4:	rts
	nop
