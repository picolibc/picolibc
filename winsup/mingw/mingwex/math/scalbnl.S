/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

	.file	"scalbnl.S"
	.text
	.align 4
.globl _scalbnl
	.def	_scalbnl;	.scl	2;	.type	32;	.endef
_scalbnl:
	fildl	16(%esp)
	fldt	4(%esp)
	fscale
	fstp	%st(1)
	ret

.globl _scalblnl
	.set	_scalblnl,_scalbnl
