/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

	.file	"copysign.S"
	.text
	.align 4
.globl _copysign
	.def	_copysign;	.scl	2;	.type	32;	.endef
_copysign:
	movl	16(%esp),%edx
	movl	8(%esp),%eax
	andl	$0x80000000,%edx
	andl	$0x7fffffff,%eax
	orl	%edx,%eax
	movl	%eax,8(%esp)
	fldl	4(%esp)
	ret
