/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * 
 * Removed header file dependency for use in libmingwex.a by
 *   Danny Smith <dannysmith@users.sourceforge.net>
 */
	.file	"tanf.S"
	.text
	.align 4
.globl _tanf
	.def	_tanf;	.scl	2;	.type	32;	.endef
_tanf:
	flds	4(%esp)
	fptan
	fnstsw	%ax
	testl	$0x400,%eax
	jnz	1f
	fstp	%st(0)
	ret
1:	fldpi
	fadd	%st(0)
	fxch	%st(1)
2:	fprem1
	fstsw	%ax
	testl	$0x400,%eax
	jnz	2b
	fstp	%st(1)
	fptan
	fstp	%st(0)
	ret
