	.globl	_findnext 
_findnext:
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	16(%esp),%edx
	movb	$0x1a,%ah
	int	$0x21		

	movb	$0x4f,%ah
	int	$0x21		

	popl	%edi
	popl	%esi
	popl	%ebx
	jmp	syscall_check
