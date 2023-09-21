	.text
	.section .rodata.vector
	.globl	__interrupt_vector
__interrupt_vector:
	.fill	31, 2, 0
	.word	_start
	.size	__interrupt_vector,.-__interrupt_vector
