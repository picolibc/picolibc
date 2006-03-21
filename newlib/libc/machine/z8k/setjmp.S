
#ifdef __Z8001__
	segm

	.global	_setjmp
	.globl	_longjmp

#ifdef __STD_CALL__

_setjmp:
	ldl	rr6,rr14(#4)	! get argument
	ldl	rr2,@rr14	! fetch pc
	ldl	@rr6,rr2	! save it
	ldl	rr6(#16),rr8
	ldl	rr6(#4),rr10
	ldl	rr6(#8),rr12	! remember frame pointer
	ldl	rr6(#12),rr14	! remember stack pointer
	ldk	r7,#0
	ret	t

_longjmp:
	ldl	rr4,rr14(#4)	! get first argument
	ld	r7,rr14(#8)	! get return value
	ldl	rr8,rr4(#16)
	ldl	rr10,rr4(#4)
	ldl	rr12,rr4(#8)	! restore old frame pointer
	ldl	rr14,rr4(#12)	! restore old stack pointer
	ldl	rr4,@rr4	! return address
	inc	r15,#4
	jp	@rr4

#else /* above __STD_CALL_, below not */

_setjmp:
	ldl	rr2,@rr14	! fetch pc
	ldl	@rr6,rr2	! save it
	ldl	rr6(16),rr8
	ldl	rr6(4),rr10
	ldl	rr6(8),rr12	! and the other special regs
	ldl	rr6(12),rr14
	ldk	r2,#0
	ret	t

_longjmp:
	ld	r2,r5		! get return value
	ldl	rr4,rr6(0)
	ldl	rr8,rr6(16)
	ldl	rr10,rr6(4)
	ldl	rr12,rr6(8)
	ldl	rr14,rr6(12)
	inc	r15,#4
	jp	@rr4

#endif  /* not __STD_CALL__ */
#else /* above Z8001, below Z8002 */
	unseg

#ifdef __STD_CALL__
#warning Z8002 std call not implemented!
#endif

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
	
#endif  /* Z8002 version */
