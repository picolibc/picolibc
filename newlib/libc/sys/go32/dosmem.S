/* DOSMEM.S */
/*
** Copyright (C) 1993 C.W. Sandmann
**
** This file may be freely distributed as long as the author's name remains.
** Extender environment independent way to set up Real area 1Mb access.
** Procedure takes a single argument %eax which contains the real area offset.
** After call, access may be made with "%gs:(%eax)"
*/
	.text
	.align	2
	.globl dosmemsetup
dosmemsetup:				/* no params, expected ASM call only */
	pushl	%eax
	movzwl	__core_select,%eax
	testl	%eax,%eax
	je	old_go32		/* Image run with pre-DPMI extender */
	movw	%ax,%gs			/* Use real area selector */
	popl	%eax			/* Plus real offset */
	andl	$0x0fffffff,%eax	/* Clear any linear access bits */
	ret
	.align	2,0x90
old_go32:
	push	%ds
	pop	%gs			/* Use arena selector */
	popl	%eax
	orl	$0xe0000000,%eax	/* Plus linear access area */
	ret

	.align	2
	.globl _dosmemget
_dosmemget:				/* long offset, long len, long *buf */
	push	%gs
	movl	8(%esp),%eax		/* offset */
	call	dosmemsetup
	movl	12(%esp),%ecx		/* length */
	movl	16(%esp),%edx		/* arena offset */
	pushl	%esi
	pushl	%edi
	movl	%eax,%esi
	movl	%edx,%edi
	push	%ds
	push	%es
	push	%ds
	pop	%es
	push	%gs
	pop	%ds
	cld
	rep
	movsb				/* move ECX bytes from Real area */
	pop	%es
	pop	%ds
	popl	%edi
	popl	%esi
	pop	%gs
	ret

	.align	2
	.globl _dosmemput
_dosmemput:				/* long *buf, long len, long offset */
	push	%gs
	movl	16(%esp),%eax		/* offset */
	call	dosmemsetup
	movl	12(%esp),%ecx		/* length */
	movl	8(%esp),%edx		/* arena offset */
	pushl	%esi
	pushl	%edi
	movl	%eax,%edi
	movl	%edx,%esi
	push	%es
	push	%gs
	pop	%es
	cld
	rep
	movsb				/* move ECX bytes to Real area */
	pop	%es
	popl	%edi
	popl	%esi
	pop	%gs
	ret

	.align	2			/* 8(bp)    12(bp)   16(bp)    20(bp)    24(bp) */
	.globl	_movedata		/* src_sel, src_ofs, dest_sel, dest_ofs, len    */
_movedata:
	pushl	%ebp
	movl	%esp,%ebp
	pushw	%ds
	pushw	%es
	pushl	%esi
	pushl	%edi

	movl	8(%ebp),%eax
	movw	%ax,%ds
	movl	12(%ebp),%esi

	movl	16(%ebp),%eax
	movw	%ax,%es
	movl	20(%ebp),%edi

	movl	24(%ebp),%ecx
	pushl	%ecx
	shrl	$2,%ecx
	jcxz	no_big_move
	rep
	movsl
no_big_move:
	popl	%ecx
	andl	$3,%ecx
	jcxz	no_little_move
	rep
	movsb
no_little_move:

	popl	%edi
	popl	%esi
	popw	%es
	popw	%ds
	leave
	ret

