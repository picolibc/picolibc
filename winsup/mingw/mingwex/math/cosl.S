/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 * Removed glibc header dependancy by Danny Smith
 *  <dannysmith@users.sourceforge.net>
 */
	.file	"cosl.S"
	.text
	.align 4
.globl _cosl
	.def	_cosl;	.scl	2;	.type	32;	.endef
_cosl:
	fldt	4(%esp)
	fcos
	fnstsw	%ax
	testl	$0x400,%eax
	jnz	1f
	ret
1:	fldpi
	fadd	%st(0)
	fxch	%st(1)
2:	fprem1
	fnstsw	%ax
	testl	$0x400,%eax
	jnz	2b
	fstp	%st(1)
	fcos
	ret
