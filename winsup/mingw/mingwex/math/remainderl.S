/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 * Removed header file dependency for use in libmingwex.a by
 *   Danny Smith <dannysmith@users.sourceforge.net>
 */

	.file	"remainderl.S"
	.text
	.align 4
.globl _remainderl
	.def	_remainderl;	.scl	2;	.type	32;	.endef
_remainderl:
	fldt	16(%esp)
	fldt	4(%esp)
1:	fprem1
	fstsw	%ax
	sahf
	jp	1b
	fstp	%st(1)
	ret
