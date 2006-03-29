
/*
**	Called as start(argc, argv, envp)
*/

/*	gs:edx points to prog_info structure.  All other registers are OBSOLETE
**	but included for backwards compatibility
*/

/* These symbols are for global constructors and destructors */
#if 0
	.section	.ctor
	.globl		___go32_first_ctor
___go32_first_ctor:
	.section	.dtor
	.globl		___go32_last_ctor
___go32_last_ctor:
	.globl		___go32_first_dtor
___go32_first_dtor:
	.data
	.globl		___go32_last_dtor
___go32_last_dtor:
#endif
	.text
	.globl	_start
_start:
	.globl	start
start:
#ifdef EMU387
	pusha
	push	%gs
#endif
	movl	%eax,__hard_master
	movl	%esi,___pid
	movl	%edi,___transfer_buffer
	movl	%ebx,_ScreenPrimary
	movl	%ebp,_ScreenSecondary

	cmpl	$0, %edx
	je	Lcopy_none
	movw	%gs,%cx
	movw	%ds,%ax
	cmpw	%cx,%ax
	je	Lcopy_none

	movl	%gs:(%edx), %ecx
	cmpl	__go32_info_block, %ecx
	jbe	Lcopy_less
	movl	__go32_info_block, %ecx
Lcopy_less:
	movl	$__go32_info_block, %edi
	addl	$3, %ecx
	andl	$0xfffffffc, %ecx
	movl	%ecx, (%edi)
	addl	$4, %edi
	addl	$4, %edx
	subl	$4, %ecx
Lcopy_more:
	movl	%gs:(%edx), %eax
	movl	%eax, (%edi)
	addl	$4, %edx
	addl	$4, %edi
	subl	$4, %ecx
	jnz	Lcopy_more

	movl	__go32_info_block+4, %eax
	movl	%eax, _ScreenPrimary
	movl	__go32_info_block+8, %eax
	movl	%eax, _ScreenSecondary
/* Backward compatibility - do not copy this one!
**	movl	__go32_info_block+12, %eax
**	movl	%eax, ___transfer_buffer
*/
	movl	__go32_info_block+20, %eax
	movl	%eax, ___pid
	movl	__go32_info_block+24, %eax
	movl	%eax, __hard_master

	jmp	Lcopy_done

Lcopy_none:
	movl	%ebx,__go32_info_block+4
	movl	%ebp,__go32_info_block+8
	movl	%edi,__go32_info_block+12
	movl	$4096,__go32_info_block+16
	movl	%esi,__go32_info_block+20
	movl	%eax,__go32_info_block+24
	movl	$28, __go32_info_block
Lcopy_done:

#ifndef EMU387
	call	__setstack
#endif
	xorl	%esi,%esi
	xorl	%edi,%edi
	xorl	%ebp,%ebp
	xorl	%ebx,%ebx

	movl	%esp,%ebx
#ifdef MAKE_GCRT0
	call	mcount_init /* initialize the profiler */
#endif
	movl	8(%ebx),%eax
	pushl	%eax
	movl	%eax,_environ
	pushl	4(%ebx)
	pushl	(%ebx)
	call	___main
	call	_main
	addl	$12,%esp
#ifdef EMU387
	pop	%gs
	popa
#else
	pushl	%eax
	call	_exit

exit_again:
	movl	$0x4c00,%eax
	int	$0x21
	jmp	exit_again
#endif

	ret


#ifdef MAKE_GCRT0
	.globl	__exit
__exit:
	call	mcount_write /* make sure we dump the output */
exit_again2:
	movb	4(%esp),%al
	movb	$0x4c,%ah
	int	$0x21
	jmp	exit_again2

/* Here is where we initialize the timer interrupt - specific to go32 */
/* In this case, the timer calls mcount_isr */
	.globl	mcount_isr_init
mcount_isr_init:
	movw	__go32_info_block+36, %ax	/* run mode */
	cmp	$1,%ax
	jb	skip_mcount
	cmp	$3,%ax
	ja	skip_mcount
	
	movw	$16,%ax
	movw	%ax,%gs

	movzbl	__hard_master,%eax	/* timer is on irq 0 */
	shll	$3,%eax /* times 8 bpv */
/*	movl	$960,%eax	   vector 0x78 * 8 bpv */
	movw	%gs:(%eax),%cx
	movw	%cx,mc_chain
	movw	%gs:6(%eax),%cx
	movw	%cx,mc_chain_hi
	movw	%gs:2(%eax),%cx
	movw	%cx,mc_chain_sel

	movl	$mcount_isr,%ecx
	movw	%cx,%gs:(%eax)
	movw	$0xd8,%gs:2(%eax)	/* selector 27 == 32-bit code */
	movw	$0x8f00,%gs:4(%eax)
	rorl	$16,%ecx
	movw	%cx,%gs:6(%eax)
	movw	%ds,%ax
	movw	%ax,%gs
skip_mcount:
	movl	mcount_histogram,%eax
	movl	$1,(%eax)
	ret

/* Obtain the PC where we interrupted, and bump the histogram.  We should  */
/* do error checking here, but we don't.  This routine is specific to go32 */
/* in some spots */
mcount_isr:
	pushl	%eax
	cmpl	$1,mcount_skip
	je	L0
	movl	4(%esp),%eax /* get the PC */
	subl	$0x1020,%eax /* to fit in low..high */
	andl	$0xfffffffc,%eax
	shrl	$1,%eax /* now points to one 4-byte entry */
	addl	mcount_histogram,%eax
	incw	(%eax)
L0:
	popl	%eax
	ljmp	mc_chain /* chain to the next timer vector */
	iret
#endif

	.data

	.globl	_environ
_environ:
	.long	0

	.globl	___pid
___pid:
	.long	42

	.globl	___transfer_buffer
___transfer_buffer:
	.long	0

	.globl	_ScreenPrimary
_ScreenPrimary:
	.long	0

	.globl	_ScreenSecondary
_ScreenSecondary:
	.long	0

	.globl	__hard_master
	.globl	__hard_slave
	.globl	__core_select
__hard_master:
	.byte	0
__hard_slave:
	.byte	0
__core_select:
	.short	0

#ifdef MAKE_GCRT0
mc_chain:
	.short	0
mc_chain_hi:
	.short	0
mc_chain_sel:
	.short	0
#endif

	
