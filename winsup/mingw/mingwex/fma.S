	.file	"fma.S"
	.text
	.align 2
	.p2align 4,,15
.globl _fma
	.def	_fma;	.scl	2;	.type	32;	.endef
_fma:
	fldl	4(%esp)
	fmull	12(%esp)
	fldl	20(%esp)
	faddp
	ret
