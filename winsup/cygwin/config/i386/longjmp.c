/* longjmp.c

   Copyright 1996, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __i386__
#if 1
asm ("	.globl	_longjmp          \n"
"_longjmp:                        \n"
"	pushl	%ebp              \n"
"	movl	%esp,%ebp	  \n"
"	movl	8(%ebp),%edi	  \n"
"	movl	12(%ebp),%eax	  \n"
"	testl	%eax,%eax	  \n"
"	jne	0f		  \n"
"	incl	%eax		  \n"
"0:				  \n"
"	movl	%eax,0(%edi)	  \n"
"	movl	24(%edi),%ebp	  \n"
"	pushfl			  \n"
"	popl	%ebx		  \n"
"	movw	42(%edi),%ax	  \n"
"	movw	%ax,%ss		  \n"
"	movl	28(%edi),%esp	  \n"
"	pushl	32(%edi)	  \n"
"       pushl	%ebx		  \n"
"	movw	36(%edi),%ax	  \n"
"	movw	%ax,%es		  \n"
#if 0
/* fs is a system register in windows; don't muck with it */
"	movw	38(%edi),%ax	  \n"
"	movw	%ax,%fs		  \n"
#endif
"	movw	40(%edi),%ax	  \n"
"	movw	%ax,%gs		  \n"
"	movl	0(%edi),%eax	  \n"
"	movl	4(%edi),%ebx	  \n"
"	movl	8(%edi),%ecx	  \n"
"	movl	12(%edi),%edx	  \n"
"	movl	16(%edi),%esi	  \n"
"	movl	20(%edi),%edi	  \n"
"	popfl			  \n"
"	ret			  \n");
#endif

#endif /* __i386__ */
