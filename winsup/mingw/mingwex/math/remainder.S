/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

	.file	"remainder.S"
	.text
	.align 4
.globl _remainder
	.def	_remainder;	.scl	2;	.type	32;	.endef
_remainder:
	fldl	12(%esp)
	fldl	4(%esp)
1:	fprem1
	fstsw	%ax
	sahf
	jp	1b
	fstp	%st(1)
	ret
