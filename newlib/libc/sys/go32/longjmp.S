/* This is file LONGJMP.S */
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

	.globl	_longjmp /* jmp_buf, int */
_longjmp:
	pushl	%ebp
	movl	%esp,%ebp

	movl	8(%ebp),%edi	/* get jmp_buf */
	movl	12(%ebp),%eax	/* store retval in j->eax */
	testl	%eax,%eax
	jne 0f
	incl %eax
0:
	movl	%eax,0(%edi)

	movl	24(%edi),%ebp

	pushfl			/* get flags so will only re-enable */
	popl	%ebx		/* interrupts if they were previously */
				/* enabled */

	cli
	movw	42(%edi),%ax
	movw	%ax,%ss
	movl	28(%edi),%esp
	
	pushl	32(%edi)	/* for a ret! */

        pushl	%ebx		/* save flags that contain previous */
				/* interrupt state */

	movw	36(%edi),%ax
	movw	%ax,%es
	movw	38(%edi),%ax
	movw	%ax,%fs
	movw	40(%edi),%ax
	movw	%ax,%gs
	movl	0(%edi),%eax
	movl	4(%edi),%ebx
	movl	8(%edi),%ecx
	movl	12(%edi),%edx
	movl	16(%edi),%esi
	movl	20(%edi),%edi

	popfl			/* restore previous interrupt state */

	ret			/* actually jump to new eip */
