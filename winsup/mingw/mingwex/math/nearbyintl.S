/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adaptedfor use as nearbyint by Ulrich Drepper <drepper@cygnus.com>.
 *
 * Removed header file dependency for use in libmingwex.a by
 *   Danny Smith <dannysmith@users.sourceforge.net>
 */

	.file	"nearbyintl.S"
	.text
	.align 4
.globl _nearbyintl
	.def	_nearbyintl;	.scl	2;	.type	32;	.endef
_nearbyintl:
	fldt	4(%esp)
	pushl	%eax
	pushl	%ecx
	fnstcw	(%esp)
	movl	(%esp), %eax
	orl	$0x20, %eax
	movl	%eax, 4(%esp)
	fldcw	4(%esp)
	frndint
	fclex
	fldcw	(%esp)
	popl	%ecx
	popl	%eax
	ret
