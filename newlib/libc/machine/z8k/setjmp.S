
#ifdef __Z8001__
	segm

	.global	_setjmp
_setjmp:
	ldl	rr2,@rr14	! fetch pc
	ldl	@rr6,rr2	! save it
	ldl	rr6(4),rr10
	ldl	rr6(8),rr12	! and the other special regs
	ldl	rr6(12),rr14
	ldk	r2,#0
	ret	t
	
	.globl	_longjmp

_longjmp:
	ld	r2,r5		! get return value
	ldl	rr4,rr6(0)
	ldl	rr10,rr6(4)
	ldl	rr12,rr6(8)
	ldl	rr14,rr6(12)
	jp	@rr4
#else
	unseg

	.global	_setjmp
_setjmp:
	ld	r2,@r15	! fetch pc
	ld	@r7,r2	! save it
	ldl	r7(4),rr10
	ldl	r7(8),rr12	! and the other special regs
	ldl	r7(12),rr14
	ldk	r2,#0
	ret	t
	
	.globl	_longjmp

_longjmp:
	ld	r2,r6		! get return value
	ld	r4,@r7
	ldl	rr10,rr7(4)
	ldl	rr12,rr7(8)
	ldl	rr14,rr7(12)
	jp	@rr4
	
#endif


