/* This is file SETJMP.S */
/*
** Copyright (C) 1993 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
**	jmp_buf:
**	 eax ebx ecx edx esi edi ebp esp eip es fs gs ss
**	 0   4   8   12  16  20  24  28  32  36 38 40 42
*/

	.globl	_setjmp /* jmp_buf */
_setjmp:
	pushl	%ebp
	movl	%esp,%ebp

	pushl	%edi
	movl	8(%ebp),%edi

	movl	%eax,0(%edi)
	movl	%ebx,4(%edi)
	movl	%ecx,8(%edi)
	movl	%edx,12(%edi)
	movl	%esi,16(%edi)

	movl	-4(%ebp),%eax
	movl	%eax,20(%edi)

	movl	0(%ebp),%eax
	movl	%eax,24(%edi)

	movl	%esp,%eax
	addl	$12,%eax
	movl	%eax,28(%edi)
	
	movl	4(%ebp),%eax
	movl	%eax,32(%edi)

	movw	%es, %ax
	movw	%ax, 36(%edi)
	movw	%fs, %ax
	movw	%ax, 38(%edi)
	movw	%gs, %ax
	movw	%ax, 40(%edi)
	movw	%ss, %ax
	movw	%ax, 42(%edi)

	popl	%edi
	movl	$0,%eax
	leave
	ret
	
