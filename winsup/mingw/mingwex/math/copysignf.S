/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

	.file	"copysignf.S"
	.text
	.align 4
.globl _copysignf
	.def	_copysignf;	.scl	2;	.type	32;	.endef
_copysignf:
	movl	8(%esp),%edx
	movl	4(%esp),%eax
	andl	$0x80000000,%edx
	andl	$0x7fffffff,%eax
	orl	%edx,%eax
	movl	%eax,4(%esp)
	flds	4(%esp)
	ret
