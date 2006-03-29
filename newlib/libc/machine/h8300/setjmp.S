	.file "setjmp.S"

#ifdef __H8300H__
	.h8300h
#endif

#ifdef __H8300S__
	.h8300s
#endif

	.section .text
	.align	2
	.global	_setjmp
_setjmp:
#ifdef __H8300H__
	mov.l	er7,@er0
	mov.l	er6,@(4,er0)
	mov.l	er5,@(8,er0)
	mov.l	er4,@(12,er0)
	mov.l	@sp,er1
	mov.l	er1,@(16,er0)
#else
#ifdef __H8300S__
	mov.l	er7,@er0
	mov.l	er6,@(4,er0)
	mov.l	er5,@(8,er0)
	mov.l	er4,@(12,er0)
	mov.l	@sp,er1
	mov.l	er1,@(16,er0)
#else
	mov.w	r7,@r0
	mov.w	r6,@(2,r0)
	mov.w	r5,@(4,r0)
	mov.w	r4,@(6,r0)
	mov.w   @sp,r1
	mov.w	r1,@(8,r0)
#endif
#endif
	sub.w	r0,r0
	rts

	.global	_longjmp
_longjmp:
#ifdef __H8300H__
	mov.l	@er0+,er7
	mov.l	@er0+,er6
	mov.l	@er0+,er5
	mov.l	@er0+,er4
	mov.l	@er0,er2
	mov.l	er2,@sp
#else
#ifdef __H8300S__
	mov.l	@er0+,er7
	mov.l	@er0+,er6
	mov.l	@er0+,er5
	mov.l	@er0+,er4
	mov.l	@er0,er2
	mov.l	er2,@sp
#else
	mov.w	@r0+,r7
	mov.w	@r0+,r6
	mov.w	@r0+,r5
	mov.w	@r0+,r4
	mov.w	@r0,r2
	mov.w	r2,@sp
#endif
#endif
	mov.w	r1,r0
	bne	.L1
	mov.w	#1,r0
.L1:
	rts
