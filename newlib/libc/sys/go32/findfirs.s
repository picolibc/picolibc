	.globl	_findfirst 
_findfirst:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	20(%esp),%edx
	movb	$0x1a,%ah
	int	$0x21		

	movl	16(%esp),%edx
	movl	24(%esp),%ecx
	movb	$0x4e,%ah
	int	$0x21		

	popl	%edi
	popl	%esi
	popl	%ebx
	jmp	syscall_check
