/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

	.file	"remainderf.S"
	.text
	.align 4
.globl _remainder
	.def	_remainderf;	.scl	2;	.type	32;	.endef
_remainderf:
	flds	8(%esp)
	flds	4(%esp)
1:	fprem1
	fstsw	%ax
	sahf
	jp	1b
	fstp	%st(1)
	ret
