	.file	"fmaf.S"
	.text
	.align 2
	.p2align 4,,15
.globl _fmaf
	.def	_fmaf;	.scl	2;	.type	32;	.endef
_fmaf:
	flds	4(%esp)
	fmuls	8(%esp)
	flds	12(%esp)
	faddp
	ret
