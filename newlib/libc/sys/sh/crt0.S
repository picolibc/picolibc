	.section .text
	.global	start
start:
	mov.l	stack_k,r15

	! zero out bss
	mov.l	edata_k,r0
	mov.l	end_k,r1
	mov	#0,r2
start_l:
	 mov.l	r2,@r0
	add	#4,r0
	cmp/ge	r0,r1
	bt	start_l

#if defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(__SH4_SINGLE_ONLY)
	mov.l set_fpscr_k, r1
	jsr @r1
	mov #0,r4
	lds r3,fpscr
#endif /*  defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(__SH4_SINGLE_ONLY__) */

	! call the mainline	
	mov.l	main_k,r0
	jsr	@r0
	or	r0,r0

	! call exit
	mov	r0,r4
	mov.l	exit_k,r0
	jsr	@r0
	or	r0,r0

	.align 2
#if defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(__SH4_SINGLE_ONLY__)
set_fpscr_k:
	.long	___set_fpscr
#endif /*  defined (__SH3E__) || defined(__SH4_SINGLE__) || defined(__SH4__) || defined(SH4_SINGLE_ONLY) */
stack_k:
	.long	_stack	
edata_k:
	.long	_edata
end_k:
	.long	_end
main_k:
	.long	_main
exit_k:
	.long	_exit

#ifdef __ELF__
	.section .stack,"aw"
#else
	.section .stack
#endif
_stack:	.long	0xdeaddead
