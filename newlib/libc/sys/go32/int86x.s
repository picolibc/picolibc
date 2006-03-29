	.data
ds:
	.word	0
es:
	.word	0
fs:
	.word	0
gs:
	.word	0

	.globl	int86
int86:
	.byte	0x2e
	push	ds
	pop	%ds
	.byte	0x2e
	push	es
	pop	%es
	.byte	0x2e
	push	fs
	pop	%fs
	.byte	0x2e
	push	gs
	pop	%gs

	.byte	0xcd
int86_vec:
	.byte	0x03
	ret

	.text
	.globl	_int86x
_int86x:
	movl	16(%esp), %eax

	movw	2(%eax), %cx
	movw	%cx, ds
	movw	4(%eax), %cx
	movw	%cx, es
	movw	6(%eax), %cx
	movw	%cx, fs
	movw	8(%eax), %cx
	movw	%cx, gs

	jmp	int86_common

	.globl	_int86
_int86:
	movw	%ds, %ax
	movw	%ax, ds
	movw	%ax, es
	movw	%ax, fs
	movw	%ax, gs
	jmp	int86_common

int86_common:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushf

	movl	8(%ebp),%eax
	movb	%al,int86_vec

	movl	12(%ebp),%eax
	movl	4(%eax),%ebx
	movl	8(%eax),%ecx
	movl	12(%eax),%edx
	movl	16(%eax),%esi
	movl	20(%eax),%edi
	movl	(%eax),%eax

	push	%ds
	push	%es
	call	int86
	pop	%es
	pop	%ds

	pushf
	pushl	%eax
	movl	%esp,%ebp
	addl	$24,%ebp
	movl	16(%ebp),%eax
	popl	(%eax)
	movl	%ebx,4(%eax)
	movl	%ecx,8(%eax)
	movl	%edx,12(%eax)
	movl	%esi,16(%eax)
	movl	%edi,20(%eax)
	popl	%ebx	/* flags */
	movl	%ebx,28(%eax)
	andl	$1,%ebx
	movl	%ebx,24(%eax)
	movl	(%eax),%eax

	popf
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret
