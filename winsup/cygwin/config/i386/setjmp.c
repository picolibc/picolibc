/* setjmp.c

   Copyright 1996, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __i386__

#if 1
asm("	.globl	_setjmp			\n"
"_setjmp:				\n"
"	pushl	%ebp			\n"
"	movl	%esp,%ebp		\n"
"	pushl	%edi			\n"
"	movl	8(%ebp),%edi		\n"
"	movl	%eax,0(%edi)		\n"
"	movl	%ebx,4(%edi)		\n"
"	movl	%ecx,8(%edi)		\n"
"	movl	%edx,12(%edi)		\n"
"	movl	%esi,16(%edi)		\n"
"	movl	-4(%ebp),%eax		\n"
"	movl	%eax,20(%edi)		\n"
"	movl	0(%ebp),%eax		\n"
"	movl	%eax,24(%edi)		\n"
"	movl	%esp,%eax		\n"
"	addl	$12,%eax		\n"
"	movl	%eax,28(%edi)		\n"
"	movl	4(%ebp),%eax		\n"
"	movl	%eax,32(%edi)		\n"
"	movw	%es, %ax		\n"
"	movw	%ax, 36(%edi)		\n"
"	movw	%fs, %ax		\n"
"	movw	%ax, 38(%edi)		\n"
"	movw	%gs, %ax		\n"
"	movw	%ax, 40(%edi)		\n"
"	movw	%ss, %ax		\n"
"	movw	%ax, 42(%edi)		\n"
"	popl	%edi			\n"
"	movl	$0,%eax			\n"
"	leave				\n"
"	ret				\n");
#endif

#endif /* __i386__ */
